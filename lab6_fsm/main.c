// main.c - Wall Follower FSM Application
// Include all necessary headers for drivers, stdint, etc.
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>
#include "gpio.h"
#include "motor.h"
#include "sensor.h"

// --- FSM and Robot Parameters ---
typedef enum { FORWARD, TURN_LEFT, TURN_RIGHT } RobotState;
#define TARGET_DISTANCE_CM 15
#define DISTANCE_THRESHOLD_CM 3
#define MOTOR_SPEED 200 // Speed from 0-255

void wall_follower_fsm(uint16_t distance_cm) {
    static RobotState currentState = FORWARD;

    // 1. State Transition Logic
    switch (currentState) {
        case FORWARD:
            // If distance is too small, change state to TURN_LEFT
            if (distance_cm < (TARGET_DISTANCE_CM - DISTANCE_THRESHOLD_CM)){
                currentState = TURN_LEFT;
            // If distance is too large, change state to TURN_RIGHT 
            } else if (distance_cm > (TARGET_DISTANCE_CM + DISTANCE_THRESHOLD_CM)){ 
                currentState = TURN_RIGHT;
            }
            break;
        case TURN_LEFT:
            // If distance is greater than target, change state back to FORWARD
            if (distance_cm > TARGET_DISTANCE_CM){
                currentState = FORWARD;
            }
            break;
        case TURN_RIGHT:
            // If distance is less than target, change state back to FORWARD
            if (distance_cm < TARGET_DISTANCE_CM){
                currentState = FORWARD;
            }
            break;
    }

    // 2. State Action Logic
    switch (currentState) {
        case FORWARD:
            motor_set_speed(MOTOR_SPEED);
            motor_forward();
            break;
        // Implement actions for TURN_LEFT (pivot away from wall)
        case TURN_LEFT:
            motor_set_speed(MOTOR_SPEED);
            motor_turn_left();
            break;
        // Implement actions for TURN_RIGHT (pivot towards wall)
        case TURN_RIGHT:
            motor_set_speed(MOTOR_SPEED);
            motor_turn_right();
            break;
    }
}

int main(void) {
    // Initialize all drivers (motor_init, sensor_init) and enable global interrupts (sei).
    motor_init();
    sensor_init();
    sei();

    while (1) {
        // This is our non-blocking superloop
        if (!sensor_is_measurement_complete()) {
            sensor_trigger();
        }
        
        _delay_ms(50); // Slow down the loop to a reasonable sensing rate

        if (sensor_is_measurement_complete()) {
            uint16_t distance_cm = sensor_get_pulse_width() / 116;
            wall_follower_fsm(distance_cm);
            // IMPORTANT: Reset the flag for the next measurement!
            sensor_reset_measurement();
        }
    }
}