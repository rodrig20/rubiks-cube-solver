#ifndef BASEMOTOR_HPP
#define BASEMOTOR_HPP

#include "Motor.hpp"

class BaseMotor : public Motor {
   public:
    using Motor::Motor;
    void turn_90(int clockwise);
    void turn_180(int clockwise);
};

#endif