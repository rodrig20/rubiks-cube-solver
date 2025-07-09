#ifndef ROBOT_HPP
#define ROBOT_HPP
#include <Adafruit_PWMServoDriver.h>

#include <string>
#include <unordered_map>

#include "motors/BaseMotor.hpp"
#include "camera/Camera.hpp"
#include "network/CubeServer.hpp"
#include "motors/GrabberMotor.hpp"
#include "solver/Solver.hpp"

using namespace std;

enum class MotorMove { Grabber, Base, Null };

const unordered_map<string, MotorMove> moveToMotorMove = {
    {"S", MotorMove::Grabber}, {"U", MotorMove::Grabber},
    {"L", MotorMove::Grabber}, {"P", MotorMove::Grabber},
    {"A", MotorMove::Base},    {"A'", MotorMove::Base},
    {"T", MotorMove::Base},    {"T'", MotorMove::Base},
    {"D", MotorMove::Base},    {"D'", MotorMove::Base},
    {"F", MotorMove::Base},    {"I", MotorMove::Base},
    // C1 a C6 não movimentam motores, não precisam de enum aqui
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
    Color colors[54];
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
    void virtual_camera(int face);
    void simplify();

   public:
    int has_state = 0;
    Solver *cube = nullptr;
    Robot();
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
