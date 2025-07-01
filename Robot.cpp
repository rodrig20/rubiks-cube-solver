#include "Robot.hpp"

#include <Arduino.h>
#include <Wire.h>

#include <iostream>
#include <sstream>
#include <string>

#include "BaseMotor.hpp"
#include "Camera.hpp"
#include "GrabberMotor.hpp"
#include "Solver.hpp"

// Inicia a comunicação I2C
Adafruit_PWMServoDriver* Robot::initI2C() {
    Adafruit_PWMServoDriver* new_pwm = new Adafruit_PWMServoDriver(0x40);
    Wire.begin(14, 15);
    new_pwm->begin();
    new_pwm->setPWMFreq(50);
    return new_pwm;
}

// Move os dois motores para a posição padrão
void Robot::init_config() {
    base->to_default();
    grabber->to_default();
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
        this->move_list += "S ";
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

// Construtor
Robot::Robot() {
    this->cube = new Solver(Solver::solved_string());
    this->pwm = initI2C();

    // Inicializar motores/câmara
    if (!pwm) {
        return;
    }
    this->cam = new Camera();
    this->base = new BaseMotor(this->pwm, 0);
    this->grabber = new GrabberMotor(this->pwm, 1);

    init_config();

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
    init_config();
    this->cube = new Solver(new_state);
    this->cube_state = "UFRBLD";
}

// Função chamada em loop para minimizar bloqueios na UI
void Robot::run() {
    //  Remover espaços iniciais
    size_t start = move_list.find_first_not_of(' ');
    if (start != std::string::npos)
        move_list = move_list.substr(start);
    else
        move_list.clear();  // A string é só espaços

    if (move_list.empty()) return;  // Nada para fazer

    // Encontrar o fim da primeira palavra
    size_t end = move_list.find(' ');

    // Extrair o primeiro movimento
    std::string move_id;
    if (end != std::string::npos) {
        move_id = move_list.substr(0, end);
        move_list = move_list.substr(end + 1);
    } else {
        move_id = move_list;
        move_list.clear();
    }

    // Traduzir movimento para função
    if (move_id == "S")
        grabber->spin(1);
    else if (move_id == "L")
        grabber->to_lock();
    else if (move_id == "P")
        grabber->to_default();
    else if (move_id == "A'")
        base->turn_90_aligned(0);
    else if (move_id == "A")
        base->turn_90_aligned(1);
    else if (move_id == "T'")
        base->turn_90(0);
    else if (move_id == "T")
        base->turn_90(1);
    else if (move_id == "D'")
        base->turn_180(0);
    else if (move_id == "D")
        base->turn_180(1);
}

// Desconstrutor
Robot::~Robot() {
    delete pwm;
    delete cube;
    delete base;
    delete grabber;
    delete cam;
}