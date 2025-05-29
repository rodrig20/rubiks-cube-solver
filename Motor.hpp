#ifndef MOTOR_HPP
#define MOTOR_HPP

#include <Adafruit_PWMServoDriver.h>

#define SERVOS_0 100
#define SERVOS_45 175
#define SERVOS_90 350
#define SERVOS_180 600

class Motor {
   private:
    Adafruit_PWMServoDriver *pwm;
    int motor_id;

   protected:
    void turn_to(int angle);

    public:
    void to_default();
    Motor(Adafruit_PWMServoDriver *pwm, int motor_id);
};

#endif