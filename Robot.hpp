#ifndef ROBOT_HPP
#define ROBOT_HPP
#include <Adafruit_PWMServoDriver.h>

#include <string>

#include "BaseMotor.hpp"
#include "Camera.hpp"
#include "GrabberMotor.hpp"
#include "Solver.hpp"
#include "CubeServer.hpp"

using namespace std;


enum class MotorMove {
    Grabber,
    Base,
    Null
};

// Forward declaration
class CubeServer;

class Robot {
   private:
    Adafruit_PWMServoDriver *pwm = nullptr;
    string cube_state = "UFRBLD";
    BaseMotor *base = nullptr;
    GrabberMotor *grabber = nullptr;
    Camera *cam = nullptr;
    CubeServer *server = nullptr;
    Adafruit_PWMServoDriver *initI2C();
    std::string move_list = "";
    MotorMove motor_move = MotorMove::Null;
    // void set_motors();
    void init_config();
    void rotate_to_side(const char side);
    void change_virtual_orientation(int changes[6]);
    void virtual_spin(int times);
    void virtual_lock();
    void virtual_lock_default();
    void virtual_turn_90(int clockwise);
    void virtual_turn_90_aligned(int clockwise);
    void virtual_turn_180(int clockwise);

    public:
    Robot();
    Solver *cube = nullptr;
    ~Robot();
    void reset();
    string solve();
    string scramble(int size);
    std::array<std::array<int, 3>, 26> piece_state();
    void get_faces();
    void turn_face(int clockwise);
    void turn_face_2();
    void virtual_move(const string move_sequence);
    void update(const string move_sequence);
    void update_state(const string new_state);
    void run();
};

#endif