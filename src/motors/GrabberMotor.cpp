#include "GrabberMotor.hpp"

// Construtor que atribui um motor_id e pwm para o I2C
GrabberMotor::GrabberMotor(Adafruit_PWMServoDriver* pwm, int motor_id)
    : Motor(pwm, motor_id) {
    to_default();
}

// Poisção com a caixa/barra para baixo
void GrabberMotor::to_lock() { turn_fast_to(275); }

// Posição natural
void GrabberMotor::to_default() { turn_fast_to(245); }

// Aplica vários spin
void GrabberMotor::spin(int times) {
    for (int i = 0; i < times; i++) {
        spin();
    }
}

// Faz o cubo girar sobre si
void GrabberMotor::spin() {
    turn_to(204);
    to_default();
}

// Mover para cima
void GrabberMotor::up() { turn_to(204); }
