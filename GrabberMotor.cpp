#include "GrabberMotor.hpp"

//MIDLE: 475

GrabberMotor::GrabberMotor(Adafruit_PWMServoDriver* pwm, int motor_id)
    : Motor(pwm, motor_id) {
    to_default();
}

void GrabberMotor::to_lock() { turn_to(275); }

void GrabberMotor::to_default() { turn_to(245); }

void GrabberMotor::spin(int times) {
    for (int i = 0; i < times; i++) {
        spin();
    }
}
void GrabberMotor::spin() {
    turn_to(204);
    to_default();
}