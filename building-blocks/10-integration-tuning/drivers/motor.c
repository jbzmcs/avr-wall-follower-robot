#include "motor.h"
#include "gpio.h"

// Direction Pins
#define DIRA_PORT    GPIO_PORT_D
#define DIRA_PIN     2   // PD2 (Arduino D2) - Motor A Direction
#define DIRB_PORT    GPIO_PORT_D
#define DIRB_PIN     4   // PD4 (Arduino D4) - Motor B Direction

// Speed (PWM) Pins 
#define PWMA_PORT    GPIO_PORT_D
#define PWMA_PIN     5   // PD5 (Arduino D5 / OC0B) - Motor A Speed
#define PWMB_PORT    GPIO_PORT_D
#define PWMB_PIN     6   // PD6 (Arduino D6 / OC0A) - Motor B Speed

static uint8_t speed_var = 0;

void motor_init(void) {
    // DIR Pin Config OUTPUT
    gpio_set_direction(DIRA_PORT, DIRA_PIN, GPIO_PIN_OUTPUT);
    gpio_set_direction(DIRB_PORT, DIRB_PIN, GPIO_PIN_OUTPUT);
    
    // PWM Pin Config as OUTPUT
    gpio_set_direction(PWMA_PORT, PWMA_PIN, GPIO_PIN_OUTPUT);
    gpio_set_direction(PWMB_PORT, PWMB_PIN, GPIO_PIN_OUTPUT);

    // Configure Timer0 for Fast PWM mode (8-bit)
    TCCR0A |= (1 << WGM01) | (1 << WGM00);   // Fast PWM Mode
    TCCR0A |= (1 << COM0A1) | (1 << COM0B1); // Clear OC0A/OC0B on compare match (Non-inverting)
    TCCR0B |= (1 << CS01)  | (1 << CS00);    // Prescaler /64
    
    motor_stop();
}

void motor_forward(void) {
    gpio_write(DIRA_PORT, DIRA_PIN, GPIO_PIN_HIGH);
    gpio_write(DIRB_PORT, DIRB_PIN, GPIO_PIN_LOW);
    
    OCR0B = speed_var;
    OCR0A = speed_var;
}

void motor_backward(void) {
    gpio_write(DIRA_PORT, DIRA_PIN, GPIO_PIN_LOW);
    gpio_write(DIRB_PORT, DIRB_PIN, GPIO_PIN_HIGH);
    
    OCR0B = speed_var; 
    OCR0A = speed_var; 
}

void motor_turn_left(void) {
    gpio_write(DIRA_PORT, DIRA_PIN, GPIO_PIN_LOW);
    gpio_write(DIRB_PORT, DIRB_PIN, GPIO_PIN_LOW);
    
    OCR0B = speed_var / 4; 
    OCR0A = speed_var; 
}

void motor_turn_right(void) {
    gpio_write(DIRA_PORT, DIRA_PIN, GPIO_PIN_HIGH);
    gpio_write(DIRB_PORT, DIRB_PIN, GPIO_PIN_HIGH);
    
    OCR0B = speed_var; 
    OCR0A = speed_var; 
}

void motor_stop(void) {
    OCR0B = 0;
    OCR0A = 0;
}

void motor_set_speeds(uint8_t speed_left, uint8_t speed_right) {
    OCR0B = speed_left;
    OCR0A = speed_right;
}