// for Activity 4.1
// motor.c - Implementation
#include "motor.h"
#include "gpio.h"

// Defined based on Activity 4.1 Table in Lab Manual
#define MOTOR_PORT_D    GPIO_PORT_D
#define MOTOR_PORT_B    GPIO_PORT_B

// Direction Pins
#define PIN_IN1         3   // PD3 (Left A)
#define PIN_IN2         4   // PD4 (Left B)
#define PIN_IN3         7   // PD7 (Right A)
#define PIN_IN4         0   // PB0 (Right B)

void motor_init(void) {
    // Configure all direction pins as OUTPUT
    gpio_set_direction(MOTOR_PORT_D, PIN_IN1, GPIO_PIN_OUTPUT);
    gpio_set_direction(MOTOR_PORT_D, PIN_IN2, GPIO_PIN_OUTPUT);
    gpio_set_direction(MOTOR_PORT_D, PIN_IN3, GPIO_PIN_OUTPUT);
    gpio_set_direction(MOTOR_PORT_B, PIN_IN4, GPIO_PIN_OUTPUT);
    
    // Initialize motors to STOP (Coast) state
    motor_stop();
}

void motor_forward(void) {
    // Left Motor Forward (IN1=High, IN2=Low)
    gpio_write(MOTOR_PORT_D, PIN_IN1, GPIO_PIN_HIGH);
    gpio_write(MOTOR_PORT_D, PIN_IN2, GPIO_PIN_LOW);
    
    // Right Motor Forward (IN3=High, IN4=Low)
    gpio_write(MOTOR_PORT_D, PIN_IN3, GPIO_PIN_HIGH);
    gpio_write(MOTOR_PORT_B, PIN_IN4, GPIO_PIN_LOW);
}

void motor_backward(void) {
    // Left Motor Backward (IN1=Low, IN2=High)
    gpio_write(MOTOR_PORT_D, PIN_IN1, GPIO_PIN_LOW);
    gpio_write(MOTOR_PORT_D, PIN_IN2, GPIO_PIN_HIGH);
    
    // Right Motor Backward (IN3=Low, IN4=High)
    gpio_write(MOTOR_PORT_D, PIN_IN3, GPIO_PIN_LOW);
    gpio_write(MOTOR_PORT_B, PIN_IN4, GPIO_PIN_HIGH);
}

void motor_turn_left(void) {
    // Left Motor Backward
    gpio_write(MOTOR_PORT_D, PIN_IN1, GPIO_PIN_LOW);
    gpio_write(MOTOR_PORT_D, PIN_IN2, GPIO_PIN_HIGH);
    
    // Right Motor Forward
    gpio_write(MOTOR_PORT_D, PIN_IN3, GPIO_PIN_HIGH);
    gpio_write(MOTOR_PORT_B, PIN_IN4, GPIO_PIN_LOW);
}

void motor_turn_right(void) {
    // Left Motor Forward
    gpio_write(MOTOR_PORT_D, PIN_IN1, GPIO_PIN_HIGH);
    gpio_write(MOTOR_PORT_D, PIN_IN2, GPIO_PIN_LOW);
    
    // Right Motor Backward
    gpio_write(MOTOR_PORT_D, PIN_IN3, GPIO_PIN_LOW);
    gpio_write(MOTOR_PORT_B, PIN_IN4, GPIO_PIN_HIGH);
}

void motor_stop(void) {
    // All Low (Coast)
    gpio_write(MOTOR_PORT_D, PIN_IN1, GPIO_PIN_LOW);
    gpio_write(MOTOR_PORT_D, PIN_IN2, GPIO_PIN_LOW);
    gpio_write(MOTOR_PORT_D, PIN_IN3, GPIO_PIN_LOW);
    gpio_write(MOTOR_PORT_B, PIN_IN4, GPIO_PIN_LOW);
}