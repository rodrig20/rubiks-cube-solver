#include "Motor.hpp"

#include <iostream>

Motor::Motor(Adafruit_PWMServoDriver *pwm, int motor_id)
    : pwm(pwm), motor_id(motor_id) {
    to_default();
    delay(1000);
}

void Motor::turn_to(int angle) {
    (*pwm).setPWM(motor_id, 0, angle);
    delay(1000);
}

void Motor::to_default() { turn_to(SERVOS_0); }