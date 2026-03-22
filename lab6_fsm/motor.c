#include "motor.h"
#include "gpio.h"

#define MOTOR_PORT_D    GPIO_PORT_D
#define MOTOR_PORT_B    GPIO_PORT_B

// Direction Pins
#define PIN_IN1         3   // PD3 (Left A)
#define PIN_IN2         4   // PD4 (Left B)
#define PIN_IN3         7   // PD7 (Right A)
#define PIN_IN4         0   // PB0 (Right B)

// Speed (PWM) Pins — must be OC0A and OC0B for Timer0 PWM
#define ENABLE_A_PORT        GPIO_PORT_D
#define ENABLE_A_PIN         6   // PD6 (OC0A) - Left Motor
#define ENABLE_B_PORT        GPIO_PORT_D
#define ENABLE_B_PIN         5   // PD5 (OC0B) - Right Motor

static uint8_t speed_var = 0;

void motor_init(void) {
    // Configure all direction pins as OUTPUT
    gpio_set_direction(MOTOR_PORT_D, PIN_IN1, GPIO_PIN_OUTPUT);
    gpio_set_direction(MOTOR_PORT_D, PIN_IN2, GPIO_PIN_OUTPUT);
    gpio_set_direction(MOTOR_PORT_D, PIN_IN3, GPIO_PIN_OUTPUT);
    gpio_set_direction(MOTOR_PORT_B, PIN_IN4, GPIO_PIN_OUTPUT);
    
    // Configure PWM enable pins as outputs
    gpio_set_direction(ENABLE_A_PORT, ENABLE_A_PIN, GPIO_PIN_OUTPUT);
    gpio_set_direction(ENABLE_B_PORT, ENABLE_B_PIN, GPIO_PIN_OUTPUT);

    // Configure Timer0 for Fast PWM mode
    TCCR0A |= (1 << WGM01) | (1 << WGM00);   // Fast PWM
    TCCR0A |= (1 << COM0A1) | (1 << COM0B1); // Non-inverting on OC0A and OC0B
    TCCR0B |= (1 << CS01)  | (1 << CS00);    // Prescaler /64

    // Start with speed 0
    OCR0A = 0;
    OCR0B = 0;

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

void motor_set_speed(uint8_t speed) {
    speed_var = speed;
    OCR0A = speed;
    OCR0B = speed;
}