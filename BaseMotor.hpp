#ifndef BASEMOTOR_HPP
#define BASEMOTOR_HPP

#include "Motor.hpp"

class BaseMotor : public Motor {
   public:
    BaseMotor(Adafruit_PWMServoDriver* pwm, int motor_id);
    void turn_90(int clockwise);
    void turn_180(int clockwise);
    void to_default() override;
};

#endif
