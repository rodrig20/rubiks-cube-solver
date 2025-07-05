#include "BaseMotor.hpp"

// Construtor que atribui um motor_id e pwm para o I2C
BaseMotor::BaseMotor(Adafruit_PWMServoDriver* pwm, int motor_id)
    : Motor(pwm, motor_id) {
    to_default();
}

// Gira o motor para a posição inicial ou 90º + um pouco
// para compensar folga da base
void BaseMotor::turn_90(int clockwise) {
    if (clockwise) {
        turn_fast_to(77 + 90);
    } else {
        turn_fast_to(-22 + 90);
    }
}
// Gira o motor para a posição inicial ou 90º
void BaseMotor::turn_90_aligned(int clockwise) {
    if (clockwise) {
        turn_fast_to(67 + 90);
    } else {
        turn_fast_to(-14 + 90);
    }
}

// Gira o motor para a posição inicial ou 180º + um pouco
// para compensar folga da base
void BaseMotor::turn_180(int clockwise) {
    if (clockwise) {
        turn_fast_to(154 + 90);
    } else {
        turn_fast_to(-14 + 90);
    }
}

// Posição natural
void BaseMotor::to_default() { turn_fast_to(-14 + 90); }
