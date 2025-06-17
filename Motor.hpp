#ifndef MOTOR_HPP
#define MOTOR_HPP

#include <Adafruit_PWMServoDriver.h>

const uint16_t SERVO_MIN = 70;   // Pulso mínimo (~0º)
const uint16_t SERVO_MAX = 520;  // Pulso máximo (~270º)

class Motor {
   private:
    int angle = -1;
    Adafruit_PWMServoDriver* pwm;
    int motor_id;
    uint16_t angleToPulse(int angle);

   protected:
    void turn_to(int angle);

   public:
    Motor(Adafruit_PWMServoDriver* pwm, int motor_id);
    virtual void to_default() = 0;
    virtual ~Motor() = default;
};

#endif
