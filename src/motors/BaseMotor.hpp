#ifndef BASEMOTOR_HPP
#define BASEMOTOR_HPP

#include "Motor.hpp"

class BaseMotor : public Motor {
   public:
    BaseMotor(Adafruit_PWMServoDriver* pwm, int motor_id);
    void turn_0();
    void turn_90(int clockwise);
    void turn_90_aligned(int clockwise);
    void turn_180(int clockwise);
    void turn_180_aligned();
    void to_default() override;
};

#endif
