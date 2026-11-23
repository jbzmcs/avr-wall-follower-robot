// Motor direction driver (L298N-style IN1-IN4 pin scheme)
// motor.h - Public API for the motor driver
#ifndef MOTOR_H
#define MOTOR_H
void motor_init(void);
void motor_forward(void);
void motor_backward(void);
void motor_turn_left(void);
void motor_turn_right(void);
void motor_stop(void);
#endif