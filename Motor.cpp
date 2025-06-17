#include "Motor.hpp"

#include <iostream>

Motor::Motor(Adafruit_PWMServoDriver *pwm, int motor_id){
    this->pwm = pwm;
    this->motor_id = motor_id;
}

void Motor::turn_to(int angle) {
    (*pwm).setPWM(motor_id, 0, angleToPulse(angle));
    delay(1000);
}

uint16_t Motor::angleToPulse(int angle) {
    return SERVO_MIN + ((angle * (SERVO_MAX - SERVO_MIN)) / 270);
}