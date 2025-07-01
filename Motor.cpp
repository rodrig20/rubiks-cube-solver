#include "Motor.hpp"

#include <iostream>


// Construtor que armazen o motor_id e pwm
Motor::Motor(Adafruit_PWMServoDriver *pwm, int motor_id) {
    this->pwm = pwm;
    this->motor_id = motor_id;
}

// Gira o motor para um angulo escolhido numa velocidade normal
void Motor::turn_to(int angle) {
    // Se o angulo for -1 ou se a diferença de angulo for muito pequena o move é realizado sem steps
    if (this->angle == -1 || abs(angle - this->angle) < 10) {
        (*pwm).setPWM(motor_id, 0, angleToPulse(angle));
        delay(23);
    } else {
        // Se o angulo for muito grande (>50) tem mais steps
        int steps;
        if (abs(angle - this->angle) > 50) {
            steps = 30;
        } else {
            steps = 20;
        }
        // Aplicar os vários steps
        float start = this->angle;
        float end = angle;
        for (int i = 1; i <= steps; i++) {
            float intermediate_angle = start + (end - start) * i / steps;
            (*pwm).setPWM(motor_id, 0, angleToPulse(intermediate_angle));
            delay(23);
        }
    }

    // Atualizar angulo
    this->angle = angle;
    delay(345);
}

// Gira o motor para um angulo escolhido numa velocidade mais rápida
void Motor::turn_fast_to(int angle) {
    // Se o angulo for -1 ou se a diferença de angulo for muito pequena o move é realizado sem steps
    if (this->angle == -1 || abs(angle - this->angle) < 10) {
        (*pwm).setPWM(motor_id, 0, angleToPulse(angle));
        delay(23);
    } else {
        // Se o angulo for muito grande (>120) tem mais steps
        int steps;
        if (abs(angle - this->angle) > 120) {
            steps = 25;
        } else {
            steps = 15;
        }

        // Aplicar os vários steps
        float start = this->angle;
        float end = angle;
        for (int i = 1; i <= steps; i++) {
            float intermediate_angle = start + (end - start) * i / steps;
            (*pwm).setPWM(motor_id, 0, angleToPulse(intermediate_angle));
            delay(23);
        }
    }

    // Atualizar angulo
    this->angle = angle;
    delay(345);
}


// Transforma angulo em pwm
uint16_t Motor::angleToPulse(int angle) {
    return SERVO_MIN + ((angle * (SERVO_MAX - SERVO_MIN)) / 270);
}