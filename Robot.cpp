#include "Robot.hpp"

#include <Arduino.h>
#include <Wire.h>
#include <esp_system.h>

#include <iostream>
#include <sstream>
#include <string>

#include "BaseMotor.hpp"
#include "Camera.hpp"
#include "CubeServer.hpp"
#include "GrabberMotor.hpp"
#include "Solver.hpp"

#define ERROR_LED_PIN 33
#define NO_ROBOT 0  // Indica se o esp32 está ligado ao robô

// Verifica se um dispositivo I2C está conectado
int device_is_present(uint8_t address) {
    Wire.beginTransmission(address);
    return (Wire.endTransmission() == 0);
}

// Pisca o led de Erro
void blink_erro(int times) {
    for (int i = 0; i < times; i++) {
        digitalWrite(ERROR_LED_PIN, LOW);
        delay(75);
        digitalWrite(ERROR_LED_PIN, HIGH);
        if (times - 1 != i) {
            delay(100);
        }
    }
}

// Inicia a comunicação I2C
Adafruit_PWMServoDriver* Robot::initI2C() {
    Wire.begin(14, 15);  // Inicializa I2C nos pinos 14 e 15

    if (!device_is_present(0x40)) {
        return nullptr;  // Ou lidar com erro como quiseres
    }

    Adafruit_PWMServoDriver* new_pwm = new Adafruit_PWMServoDriver(0x40);
    new_pwm->begin();
    new_pwm->setPWMFreq(50);
    return new_pwm;
}

// Move os dois motores para a posição padrão
void Robot::init_config() {
    virtual_lock_default();
    virtual_turn_90_aligned(0);
}

// Aplica um movimento ao cubo causado pelas rotações da base
void Robot::change_virtual_orientation(int changes[6]) {
    string temp = cube_state;
    for (int i = 0; i < 6; i++) {
        temp[i] = cube_state[changes[i]];
    }

    cube_state = temp;
}

// Aplica a uma sequencia de movimentos dos motores para uma certa face ficar
// para baixo
void Robot::rotate_to_side(const char side) {
    int side_idx;
    for (int i = 0; i < cube_state.size(); i++) {
        if (cube_state[i] == side) {
            side_idx = i;
            break;
        }
    }

    switch (side_idx) {
        // Topo
        case 0: {
            virtual_spin(2);
            int pattern0[6] = {5, 3, 2, 1, 4, 0};
            change_virtual_orientation(pattern0);
            break;
        }
        // Frente
        case 1: {
            virtual_spin(1);
            int pattern1[6] = {3, 0, 2, 5, 4, 1};
            change_virtual_orientation(pattern1);
            break;
        }
        // Direita
        case 2: {
            virtual_turn_90_aligned(1);
            virtual_spin(3);
            virtual_turn_90_aligned(0);
            int pattern2[6] = {4, 1, 0, 3, 5, 2};
            change_virtual_orientation(pattern2);
            break;
        }
        // Trás
        case 3: {
            virtual_spin(3);
            int pattern3[6] = {1, 5, 2, 0, 4, 3};
            change_virtual_orientation(pattern3);
            break;
        }
        // Esquerda
        case 4: {
            virtual_turn_90_aligned(1);
            virtual_spin(1);
            virtual_turn_90_aligned(0);
            int pattern4[6] = {2, 1, 5, 3, 0, 4};
            change_virtual_orientation(pattern4);
            break;
        }
    }
}

// Passar o movimento spin para o buffer
void Robot::virtual_spin(int times) {
    for (int i = 0; i < times; i++) {
        this->move_list += "U P ";
    }
}

// Passar o movimento lock para o buffer
void Robot::virtual_lock() { this->move_list += "L "; }

// Passar o movimento da garra para o default para o buffer
void Robot::virtual_lock_default() { this->move_list += "P "; }

// Passar o movimento em 90º alinhado para o buffer
void Robot::virtual_turn_90_aligned(int clockwise) {
    if (clockwise) {
        this->move_list += "A ";
    } else {
        this->move_list += "A' ";
    }
}

// Passar o movimento em 90º para o buffer
void Robot::virtual_turn_90(int clockwise) {
    if (clockwise) {
        this->move_list += "T ";
    } else {
        this->move_list += "T' ";
    }
}

// Passar o movimento em 180º para o buffer
void Robot::virtual_turn_180(int clockwise) {
    if (clockwise) {
        this->move_list += "D ";
    } else {
        this->move_list += "D' ";
    }
}

// Passar a captura de imagem para o buffer
void Robot::virtual_camera(int face) {
    this->move_list += "C" + std::to_string(face) + " ";
}

// Construtor
Robot::Robot() {
    pinMode(ERROR_LED_PIN, OUTPUT);
    this->cube = new Solver(Solver::solved_string());
    has_state = 1;
    this->server = new CubeServer(this, 80);

// Executa se o ESP32 está conectado ao robô
#if !NO_ROBOT
    this->pwm = initI2C();

    // Inicializar motores/câmara
    if (!pwm) {
        std::cout << "I2C não detetado. A reiniciar..." << std::endl;
        // Avisa do reinicio e volta a ligar
        blink_erro(2);
        esp_restart();
        return;
    }
    this->base = new BaseMotor(this->pwm, 0);
    this->grabber = new GrabberMotor(this->pwm, 1);

    this->cam = new Camera();
    if (!this->cam->initialized) {
        std::cout << "Câmara não detetada. A reiniciar..." << std::endl;
        // Avisa do reinicio e volta a ligar
        blink_erro(3);
        esp_restart();
        return;
    }

    init_config();

#endif

    // Pisca o LED para indicar que está pronto
    for (int i = 0; i < 3; i++) {
        analogWrite(LED_PIN, LED_BRIGHTNESS);
        delay(75);
        analogWrite(LED_PIN, 0);
        delay(75);
    }
}

// Baralha o cubo
std::string Robot::scramble(int size) {
    std::string move_sequence = cube->scramble(size);
    virtual_move(move_sequence);
    return move_sequence;
}

// Resolve o cubo
std::string Robot::solve() {
    std::string move_sequence = cube->solve();
    virtual_move(move_sequence);
    return move_sequence;
}

// Ativa a sequencia de motores para girar a face de baixo em 90º
void Robot::turn_face(int clockwise) {
    if (clockwise) {
        virtual_lock();
        virtual_turn_90(1);
        virtual_lock_default();
        virtual_turn_90_aligned(0);
        int pattern[6] = {0, 2, 3, 4, 1, 5};
        change_virtual_orientation(pattern);
    } else {
        virtual_turn_90_aligned(1);
        virtual_lock();
        virtual_turn_90(0);
        virtual_lock_default();
        virtual_turn_90_aligned(0);
        int pattern[6] = {0, 4, 1, 2, 3, 5};
        change_virtual_orientation(pattern);
    }
}

// Obtem todas as faces do cubo com a camara
void Robot::get_faces() {
#if !NO_ROBOT
    has_state = 0;
    this->move_list = "";
    init_config();
    this->cube_state = "UFRBLD";

    // Face do topo
    virtual_lock();
    virtual_camera(1);

    virtual_spin(1);

    // Face de trás
    virtual_lock();
    virtual_camera(2);

    virtual_spin(1);

    // Face de baixo
    virtual_lock();
    virtual_camera(3);

    virtual_spin(1);

    // Face da frente
    virtual_lock();
    virtual_camera(4);

    virtual_lock_default();

    virtual_turn_90_aligned(1);
    virtual_spin(1);

    // Face da direita
    virtual_lock();
    virtual_camera(5);

    virtual_spin(2);

    // Face da esquerda
    virtual_lock();
    virtual_camera(6);

    virtual_spin(1);
    virtual_turn_90_aligned(0);
    virtual_spin(1);
#else
    // Assume o estado resolvido
    update_state(Solver::solved_string());
#endif
}

// Ativa a sequencia de motores para girar a face de baixo em 180º
void Robot::turn_face_2() {
    virtual_lock();
    virtual_turn_180(1);
    virtual_lock_default();
    virtual_turn_180(0);
    int pattern[6] = {0, 3, 4, 1, 2, 5};
    change_virtual_orientation(pattern);
}

// Aplica uma string de movimentos
void Robot::virtual_move(const string move_string) {
    if (move_string == "-") return;
    stringstream ss(move_string);
    string move;
    // Traduzir movimento para função
    while (ss >> move) {
        rotate_to_side(move[0]);
        if (move.size() == 1) {
            turn_face(1);
        } else if (move[1] == '\'') {
            turn_face(0);
        } else {
            turn_face_2();
        }
    }
}

// Aplica uma string de movimentos
void Robot::update(const string move_string) {
    this->cube->move(move_string);
    virtual_move(move_string);
}

void Robot::update_state(const string new_state) {
    this->move_list = "";
#if !NO_ROBOT
    init_config();
#endif
    this->cube = new Solver(new_state);
    this->cube_state = "UFRBLD";
    has_state = 1;
}

void Robot::reset() { update_state(Solver::solved_string()); }

// Função chamada em loop para minimizar bloqueios na UI
void Robot::run() {
    server->handleClient();
// Executa se o ESP32 está conectado ao robô
#if !NO_ROBOT
    std::string move_id;
    if (motor_move == MotorMove::Null) {
        //  Remover espaços iniciais
        size_t start = move_list.find_first_not_of(' ');
        if (start != std::string::npos)
            move_list = move_list.substr(start);
        else
            move_list.clear();  // A string é só espaços
        if (!move_list.empty()) {
            // Encontrar o fim da primeira palavra
            size_t end = move_list.find(' ');

            // Extrair o primeiro movimento
            if (end != std::string::npos) {
                move_id = move_list.substr(0, end);
                move_list = move_list.substr(end + 1);
            } else {
                move_id = move_list;
                move_list.clear();
            }
            // Traduzir movimento para função
            if (move_id == "S") {
                grabber->spin(1);
                motor_move = MotorMove::Grabber;
            } else if (move_id == "U") {
                grabber->up();
                motor_move = MotorMove::Grabber;
            } else if (move_id == "L") {
                grabber->to_lock();
                motor_move = MotorMove::Grabber;
            } else if (move_id == "P") {
                grabber->to_default();
                motor_move = MotorMove::Grabber;
            } else if (move_id == "A'") {
                base->turn_90_aligned(0);
                motor_move = MotorMove::Base;
            } else if (move_id == "A") {
                base->turn_90_aligned(1);
                motor_move = MotorMove::Base;
            } else if (move_id == "T'") {
                base->turn_90(0);
                motor_move = MotorMove::Base;
            } else if (move_id == "T") {
                base->turn_90(1);
                motor_move = MotorMove::Base;
            } else if (move_id == "D'") {
                base->turn_180(0);
                motor_move = MotorMove::Base;
            } else if (move_id == "D") {
                base->turn_180(1);
                motor_move = MotorMove::Base;
            } else if (move_id == "C1") {
                Color* face = cam->get_color_face();
                for (int i = 0; i < 9; i++) {
                    colors[i].R = face[i].R;
                    colors[i].G = face[i].G;
                    colors[i].B = face[i].B;
                }
                delete face;
                face = nullptr;
            } else if (move_id == "C2") {
                Color* face = cam->get_color_face();
                for (int i = 8; i >= 0; i--) {
                    colors[3 * 9 + i].R = face[(9 - 1) - i].R;
                    colors[3 * 9 + i].G = face[(9 - 1) - i].G;
                    colors[3 * 9 + i].B = face[(9 - 1) - i].B;
                }
                delete face;
                face = nullptr;
            } else if (move_id == "C3") {
                Color* face = cam->get_color_face();
                for (int i = 0; i < 9; i++) {
                    colors[5 * 9 + i].R = face[i].R;
                    colors[5 * 9 + i].G = face[i].G;
                    colors[5 * 9 + i].B = face[i].B;
                }
                delete face;
                face = nullptr;
            } else if (move_id == "C4") {
                Color* face = cam->get_color_face();
                for (int i = 0; i < 9; i++) {
                    colors[1 * 9 + i].R = face[i].R;
                    colors[1 * 9 + i].G = face[i].G;
                    colors[1 * 9 + i].B = face[i].B;
                }
                delete face;
                face = nullptr;
            } else if (move_id == "C5") {
                Color* face = cam->get_color_face();
                int c = 0;
                for (int i = 2; i >= 0; i--) {
                    for (int j = 0; j < 3; j++) {
                        colors[2 * 9 + (i + (j * 3))].R = face[c].R;
                        colors[2 * 9 + (i + (j * 3))].G = face[c].G;
                        colors[2 * 9 + (i + (j * 3))].B = face[c].B;
                        c += 1;
                    }
                }
                delete face;
                face = nullptr;
            } else if (move_id == "C6") {
                Color* face = cam->get_color_face();
                int c = 0;
                for (int i = 2; i >= 0; i--) {
                    for (int j = 0; j < 3; j++) {
                        colors[4 * 9 + (i + (j * 3))].R = face[c].R;
                        colors[4 * 9 + (i + (j * 3))].G = face[c].G;
                        colors[4 * 9 + (i + (j * 3))].B = face[c].B;
                        c += 1;
                    }
                }
                delete face;
                face = nullptr;

                int labels[54];

                // Agrupar cores
                cam->grouping_colors(colors, labels);

                std::string cube_state = "";
                for (int i = 0; i < 54; ++i) {
                    cube_state += std::to_string(labels[i]);
                }

                this->cube = new Solver(cube_state);
                has_state = 1;
            }
        } else {
            motor_move = MotorMove::Null;
        }
    }
    if (motor_move == MotorMove::Grabber) {
        if (grabber->step()) motor_move = MotorMove::Null;

    } else if (motor_move == MotorMove::Base) {
        if (base->step()) motor_move = MotorMove::Null;
    }
#endif
}

// Desconstrutor
Robot::~Robot() {
    delete cube;
    delete server;
// Delete se o robô existir
#if !NO_ROBOT
    delete pwm;
    delete base;
    delete grabber;
    delete cam;
#endif
}
