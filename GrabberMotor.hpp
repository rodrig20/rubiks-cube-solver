#ifndef GRABBERMOTOR_HPP
#define GRABBERMOTOR_HPP

#include "Motor.hpp"

class GrabberMotor: public Motor {
    public:
        using Motor::Motor;
        void to_lock();
        void to_default();
        void spin(int times);
        void spin();
};

#endif