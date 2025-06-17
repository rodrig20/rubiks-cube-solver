#include "BaseMotor.hpp"

BaseMotor::BaseMotor(Adafruit_PWMServoDriver* pwm, int motor_id)
    : Motor(pwm, motor_id) {
    to_default();
}

void BaseMotor::turn_90(int clockwise) {
    if (clockwise) {
        turn_to(77+90);
    } else {
        turn_to(-22+90);
    }
}

void BaseMotor::turn_90_aligned(int clockwise) {
    if (clockwise) {
        turn_to(67+90);
    } else {
        turn_to(-14+90);
    }
}

void BaseMotor::turn_180(int clockwise) {
    if (clockwise) {
        turn_to(154+90);
    } else {
        turn_to(-14+90);
    }
}

void BaseMotor::to_default() {
    turn_to(-14+90);
}
