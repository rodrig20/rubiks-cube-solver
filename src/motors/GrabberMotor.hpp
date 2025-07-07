#ifndef GRABBERMOTOR_HPP
#define GRABBERMOTOR_HPP

#include "Motor.hpp"

class GrabberMotor : public Motor {
   public:
    GrabberMotor(Adafruit_PWMServoDriver* pwm, int motor_id);
    void to_lock();
    void to_default() override;
    void spin(int times);
    void spin();
    void up();
};

#endif
