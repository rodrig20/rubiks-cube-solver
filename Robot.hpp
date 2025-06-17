#ifndef ROBOT_HPP
#define ROBOT_HPP
#include <Adafruit_PWMServoDriver.h>

#include <string>

#include "BaseMotor.hpp"
#include "GrabberMotor.hpp"

using namespace std;

class Robot {
   private:
    Adafruit_PWMServoDriver *pwm = nullptr;
    string cube_state = "UFRBLD";
    BaseMotor *base = nullptr;
    GrabberMotor  *grabber = nullptr;
    Adafruit_PWMServoDriver* initI2C();
    // void set_motors();
    void default_position();
    void rotate_to_side(const char side);
    void move_virtual(int changes[6]);

   public:
    Robot();
    ~Robot();
    void turn_face(int clockwise);
    void turn_face_2();
    void move(const string move_sequence);
};

#endif