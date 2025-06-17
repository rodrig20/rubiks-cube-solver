#include "Motor.hpp"

#include <iostream>

Motor::Motor(Adafruit_PWMServoDriver *pwm, int motor_id){
    this->pwm = pwm;
    this->motor_id = motor_id;
}

void Motor::turn_to(int angle) {
    if (this->angle == -1 || abs(angle-this->angle) < 10) {
        (*pwm).setPWM(motor_id, 0, angleToPulse(angle));
        delay(20);
    } else {
        int steps = 20;
        float start = this->angle;
        float end = angle;
        for (int i = 1; i <= steps; i++) {
            float intermediate_angle = start + (end - start) * i / steps;
            (*pwm).setPWM(motor_id, 0, angleToPulse(intermediate_angle));
            delay(20);
        }
    }

    this->angle = angle;
    delay(333);
}


uint16_t Motor::angleToPulse(int angle) {
    return SERVO_MIN + ((angle * (SERVO_MAX - SERVO_MIN)) / 270);
}