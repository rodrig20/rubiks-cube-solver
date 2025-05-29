#include "GrabberMotor.hpp"

void GrabberMotor::to_lock() { turn_to(SERVOS_0); }

void GrabberMotor::to_default() { turn_to(SERVOS_45); }

void GrabberMotor::spin(int times) {
    for (int i = 0; i < times; i++) {
        spin();
    }
}
void GrabberMotor::spin() {
    turn_to(SERVOS_90);
    to_default();
}