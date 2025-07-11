#include "Motor.hpp"

#include <Arduino.h>

// Construtor que armazen o motor_id e pwm
Motor::Motor(Adafruit_PWMServoDriver *pwm, int motor_id) {
    this->pwm = pwm;
    this->motor_id = motor_id;
}

// Configura o motor girar para um angulo escolhido numa velocidade normal
void Motor::turn_to(float angle) {
    final_angle = angle;
    // Angulo inicial
    if (this->angle == -1) {
        steps_remaning = 1;
    } else {
        // Escolher o número de steps
        if (abs(angle - this->angle) > 50) {
            steps_remaning = 30;
        } else {
            steps_remaning = 20;
        }
    }
}

// Configura o motor girar para um angulo escolhido numa velocidade rápida
void Motor::turn_fast_to(int angle) {
    final_angle = angle;
    // Angulo inicial
    if (this->angle == -1) {
        steps_remaning = 1;
    } else {
        // Escolher o número de steps
        if (abs(angle - this->angle) > 180) {
            steps_remaning = 35;
        } else if (abs(angle - this->angle) > 120) {
            steps_remaning = 25;
        } else {
            steps_remaning = 15;
        }
    }
}

// Aplica um delay com millis
void Motor::dly(int millis_time) { next_move_time = millis() + millis_time; }

// Aplica o próximo passo do movimento do motor
int Motor::step() {
    // Verifica se já passou o tempo definido
    if (next_move_time < millis()) {
        // Se o angulo já chegou ao fim atualiza
        if (current_angle == final_angle) {
            // Atualiza e retorna que já acabou
            this->angle = final_angle;
            return 1;
        }

        // Verifica se o angulo é muito pequeno ou for o ultimo step
        if (abs(final_angle - this->angle) < 10 || steps_remaning <= 1) {
            // Atualiza a var da posição atual e moev até ao final
            current_angle = final_angle;
            (*pwm).setPWM(motor_id, 0, angleToPulse(final_angle));
            dly(23 + 345);
        } else {
            // Trocar a var da posição atual para o próximo passo
            current_angle += (final_angle - current_angle) / steps_remaning;
            steps_remaning--;

            (*pwm).setPWM(motor_id, 0, angleToPulse(current_angle));
            dly(23);
        }
    }

    return 0;
}

// Transforma angulo em pwm
uint16_t Motor::angleToPulse(float angle) {
    return SERVO_MIN + ((angle * (SERVO_MAX - SERVO_MIN)) / 270);
}
