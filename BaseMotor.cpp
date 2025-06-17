#include "BaseMotor.hpp"

BaseMotor::BaseMotor(Adafruit_PWMServoDriver* pwm, int motor_id)
    : Motor(pwm, motor_id) {
    to_default();
}

void BaseMotor::turn_90(int clockwise) {
    if (clockwise) {
        turn_to(81);
    } else {
        turn_to(0);
    }
}

void BaseMotor::turn_180(int clockwise) {
    if (clockwise) {
        turn_to(168);
    } else {
        turn_to(0);
    }
}

void BaseMotor::to_default() {
    turn_to(0);
}
