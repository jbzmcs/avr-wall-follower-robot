    // drivers/motor.c - Cleaned and Uncursed
    #include "motor.h"
    #include "gpio.h"

    // Direction Pins
    #define DIRA_PORT    GPIO_PORT_D
    #define DIRA_PIN     2   
    #define DIRB_PORT    GPIO_PORT_D
    #define DIRB_PIN     4   

    // Speed (PWM) Pins 
    #define PWMA_PORT    GPIO_PORT_D
    #define PWMA_PIN     5   
    #define PWMB_PORT    GPIO_PORT_D
    #define PWMB_PIN     6   

    void motor_init(void) {
        gpio_set_direction(DIRA_PORT, DIRA_PIN, GPIO_PIN_OUTPUT);
        gpio_set_direction(DIRB_PORT, DIRB_PIN, GPIO_PIN_OUTPUT);
        gpio_set_direction(PWMA_PORT, PWMA_PIN, GPIO_PIN_OUTPUT);
        gpio_set_direction(PWMB_PORT, PWMB_PIN, GPIO_PIN_OUTPUT);

        // Fast PWM Mode (8-bit), Prescaler /64
        TCCR0A |= (1 << WGM01) | (1 << WGM00);   
        TCCR0A |= (1 << COM0A1) | (1 << COM0B1); 
        TCCR0B |= (1 << CS01)  | (1 << CS00);    
        
        motor_stop();
    }

    // Notice: These now ONLY change direction. No more speed reset bugs!
    void motor_forward(void) {
        gpio_write(DIRA_PORT, DIRA_PIN, GPIO_PIN_HIGH);
        gpio_write(DIRB_PORT, DIRB_PIN, GPIO_PIN_LOW);
    }

    void motor_backward(void) {
        gpio_write(DIRA_PORT, DIRA_PIN, GPIO_PIN_LOW);
        gpio_write(DIRB_PORT, DIRB_PIN, GPIO_PIN_HIGH);
    }

    void motor_turn_left(void) {
        gpio_write(DIRA_PORT, DIRA_PIN, GPIO_PIN_LOW);
        gpio_write(DIRB_PORT, DIRB_PIN, GPIO_PIN_LOW);
    }

    void motor_turn_right(void) {
        gpio_write(DIRA_PORT, DIRA_PIN, GPIO_PIN_HIGH);
        gpio_write(DIRB_PORT, DIRB_PIN, GPIO_PIN_HIGH);
    }

    void motor_set_speeds(uint8_t speed_left, uint8_t speed_right) {
        OCR0B = speed_left;
        OCR0A = speed_right;
    }

    void motor_stop(void) {
        OCR0B = 0;
        OCR0A = 0;
    }