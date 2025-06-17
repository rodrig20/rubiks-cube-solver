#include "Robot.hpp"

#include <Arduino.h>
#include <Wire.h>

#include <iostream>
#include <sstream>
#include <string>

#include "BaseMotor.hpp"
#include "GrabberMotor.hpp"

// Inicia a comunicação I2C
Adafruit_PWMServoDriver* Robot::initI2C() {
    pwm = new Adafruit_PWMServoDriver(0x40);

    Wire.begin(14, 15);
    pwm->begin();
    pwm->setPWMFreq(50);

    return pwm;
}

// Move os dois motores para a posição padrão
void Robot::default_position() {
    base->to_default();
    grabber->to_default();
}

// Aplica um movimento ao cubo causado pelas rotações da base
void Robot::move_virtual(int changes[6]) {
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
            grabber->spin(2);
            int pattern0[6] = {5, 3, 2, 1, 4, 0};
            move_virtual(pattern0);
            break;
        }
        // Frente
        case 1: {
            grabber->spin(1);
            int pattern1[6] = {3, 0, 2, 5, 4, 1};
            move_virtual(pattern1);
            break;
        }
        // Direita
        case 2: {
            base->turn_90(1);
            grabber->spin(3);
            base->turn_90(0);
            int pattern2[6] = {4, 1, 0, 3, 5, 2};
            move_virtual(pattern2);
            break;
        }
        // Trás
        case 3: {
            grabber->spin(3);
            int pattern3[6] = {1, 5, 2, 0, 4, 3};
            move_virtual(pattern3);
            break;
        }
        // Esquerda
        case 4: {
            base->turn_90(1);
            grabber->spin(1);
            base->turn_90(0);
            int pattern4[6] = {2, 1, 5, 3, 0, 4};
            move_virtual(pattern4);
            break;
        }
    }
}

Robot::Robot()
    : pwm(initI2C()),
      base(new BaseMotor(pwm, 0)),
      grabber(new GrabberMotor(pwm, 1)) {
    if (!pwm) {
        return;
    }
    initI2C();
    default_position();
}

// Ativa a sequencia de motores para girar a face de baixo em 90º
void Robot::turn_face(int clockwise) {
    if (clockwise) {
        grabber->to_lock();
        base->turn_90(1);
        delay(1000);
        grabber->to_default();
        base->turn_90(0);
        int pattern[6] = {0, 2, 3, 4, 1, 5};
        move_virtual(pattern);
    } else {
        base->turn_90(1);
        grabber->to_lock();
        base->turn_90(0);
        grabber->to_default();
        int pattern[6] = {0, 4, 1, 2, 3, 5};
        move_virtual(pattern);
    }
}

// Ativa a sequencia de motores para girar a face de baixo em 180º
void Robot::turn_face_2() {
    grabber->to_lock();
    base->turn_180(1);
    delay(2000);
    grabber->to_default();
    base->turn_180(0);
    int pattern[6] = {0, 3, 4, 1, 2, 5};
    move_virtual(pattern);
}

// Aplica uma string de movimentos
void Robot::move(const string move_string) {
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

Robot::~Robot() {
    delete pwm;
    delete base;
    delete grabber;
}