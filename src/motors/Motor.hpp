#ifndef MOTOR_HPP
#define MOTOR_HPP

#include <Adafruit_PWMServoDriver.h>
#include <Arduino.h>

const uint16_t SERVO_MIN = 70;   // Pulso mínimo (~0º)
const uint16_t SERVO_MAX = 468;  // Pulso máximo (~270º)
const int SERVO_STEP_SIZE = 10;
const int SERVO_STEP_DELAY = 37;
const int SERVO_MOVE_DELAY = 150;

class Motor {
   private:
    float angle = -1;
    Adafruit_PWMServoDriver* pwm;
    int motor_id;
    float final_angle;
    int steps_remaning;
    float current_angle;
    unsigned long next_move_time = 0;
    void dly(int milllis_time);
    uint16_t angleToPulse(float angle);
    void turn_with_speed_to(float angle, int speed_percent);

   protected:
    void turn_slow_to(float angle);
    void turn_fast_to(float angle);

   public:
    int step();
    Motor(Adafruit_PWMServoDriver* pwm, int motor_id);
    virtual void to_default() = 0;
    virtual ~Motor() = default;
};

#endif
