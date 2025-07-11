#include "BaseMotor.hpp"

// Construtor que atribui um motor_id e pwm para o I2C
BaseMotor::BaseMotor(Adafruit_PWMServoDriver* pwm, int motor_id)
    : Motor(pwm, motor_id) {
    to_default();
}

void BaseMotor::turn_0() { turn_fast_to(0); }
void BaseMotor::turn_180_aligned() { turn_fast_to(267); }

// Gira o motor para a posição inicial ou 90º + um pouco
// para compensar folga da base
void BaseMotor::turn_90(int clockwise) {
    if (clockwise) {
        turn_fast_to(184.5);
    } else {
        turn_fast_to(73.2);
    }
}
// Gira o motor para a posição inicial ou 90º
void BaseMotor::turn_90_aligned(int clockwise) {
    if (clockwise) {
        turn_fast_to(177);
    } else {
        turn_fast_to(85);
    }
}

// Gira o motor para a posição inicial ou 180º + um pouco
// para compensar folga da base
void BaseMotor::turn_180(int clockwise) {
    if (clockwise) {
        turn_fast_to(275.3);
    } else {
        turn_fast_to(84.7);
    }
}

// Posição natural
void BaseMotor::to_default() { turn_fast_to(85); }
