#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>
#include "gpio.h"
#include "motor.h"
#include "sensor.h"

// --- FSM and Robot Parameters ---
typedef enum { FORWARD, TURN_LEFT, TURN_RIGHT } RobotState;
#define TARGET_DISTANCE_CM    15
#define DISTANCE_THRESHOLD_CM 5
#define MOTOR_SPEED           60

// Differential speed for gentle arc turns
// #define TURN_FAST  70
// #define TURN_SLOW  50

#define TURN_LEFT_FAST   75
#define TURN_LEFT_SLOW   30

#define TURN_RIGHT_FAST  80    // Lower to slow down the right turn
#define TURN_RIGHT_SLOW  30

void wall_follower_fsm(uint16_t distance_cm) {
    static RobotState currentState = FORWARD;

    // 1. State Transition Logic
    switch (currentState) {
        case FORWARD:
            if (distance_cm < (TARGET_DISTANCE_CM - DISTANCE_THRESHOLD_CM)) {
                currentState = TURN_LEFT;
            } else if (distance_cm > (TARGET_DISTANCE_CM + DISTANCE_THRESHOLD_CM)) {
                currentState = TURN_RIGHT;
            }
            break;
        case TURN_LEFT:
            if (distance_cm >= (TARGET_DISTANCE_CM - DISTANCE_THRESHOLD_CM)) {
                currentState = FORWARD;
            }
            break;
        case TURN_RIGHT:
            if (distance_cm <= (TARGET_DISTANCE_CM + DISTANCE_THRESHOLD_CM)) {
                currentState = FORWARD;
            }
            break;
    }

    // 2. State Action Logic
    switch (currentState) {
        case FORWARD:
            motor_set_speeds(MOTOR_SPEED, MOTOR_SPEED);
            break;
        case TURN_LEFT:
            // Too close to wall — steer away (slow left, fast right)
            motor_set_speeds(TURN_LEFT_SLOW, TURN_RIGHT_FAST);
            break;
        case TURN_RIGHT:
            // Too far from wall — steer toward (fast left, slow right)
            motor_set_speeds(TURN_LEFT_FAST, TURN_RIGHT_SLOW);
            break;
    }
}

int main(void) {
    // Initialize all drivers
    motor_init();
    sensor_init();
    sei();

    // Set direction once — LAFVIN board uses single-DIO per motor,
    // so turns are done via differential speed, not direction reversal.
    motor_forward();

    while (1) {
        // Non-blocking superloop
        if (!sensor_is_measurement_complete()) {
            sensor_trigger();
        }

        _delay_ms(30);

        if (sensor_is_measurement_complete()) {
            uint16_t distance_cm = sensor_get_pulse_width() / 116;

            if (distance_cm > 0 && distance_cm < 400) {
                wall_follower_fsm(distance_cm);
            }

            // Reset flag for next measurement
            sensor_reset_measurement();
        }
    }
}