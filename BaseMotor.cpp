#include "BaseMotor.hpp"

void BaseMotor::turn_90(int clockwise) {
    if (clockwise) {
        turn_to(SERVOS_90);
    } else {
        turn_to(SERVOS_0);
    }
}

void BaseMotor::turn_180(int clockwise) {
    if (clockwise) {
        turn_to(SERVOS_180);
    } else {
        turn_to(SERVOS_0);
    }
}