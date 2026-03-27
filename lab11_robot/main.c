// main.c - Lab 11: Wall Follower with Servo Obstacle Avoidance

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdio.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>
#include <avr/sleep.h>

#include "drivers/motor.h"
#include "drivers/sensor.h"
#include "drivers/uart.h"
#include "drivers/servo.h"

// --- EEPROM ---
uint16_t g_target_distance_cm = 15;
uint16_t EEMEM g_target_distance_eeprom;
#define erasedMemory 0xFFFF

// --- FSM States ---
typedef enum { IDLE, FORWARD, TURN_LEFT, TURN_RIGHT } RobotState;
volatile RobotState currentState = IDLE;

// --- Speed Parameters ---
#define MOTOR_SPEED           30    // Cruise speed (both wheels)
#define OBSTACLE_DISTANCE_CM  22    // Front obstacle threshold

void settings_load(void) {
    uint16_t stored = eeprom_read_word(&g_target_distance_eeprom);
    if (stored == erasedMemory) {
        eeprom_update_word(&g_target_distance_eeprom, g_target_distance_cm);
    } else {
        g_target_distance_cm = stored;
    }
    printf("Target distance set to %u cm\n", g_target_distance_cm);
}

// --- UART Receive Complete ISR ---
ISR(USART_RX_vect) {
    unsigned char command = UDR0;
    switch (command) {
        case 's':
            currentState = FORWARD;
            printf("\nCOMMAND: START\n");
            break;
        case 'p':
            currentState = IDLE;
            printf("\nCOMMAND: STOP\n");
            break;
        case '+':
            g_target_distance_cm++;
            printf("Target: %u cm\n", g_target_distance_cm);
            break;
        case '-':
            if (g_target_distance_cm > 1){
                g_target_distance_cm--;
            }
            printf("Target: %u cm\n", g_target_distance_cm);
            break;
        case 'w':
            eeprom_update_word(&g_target_distance_eeprom, g_target_distance_cm);
            printf("Saved %u cm to EEPROM\n", g_target_distance_cm);
            break;
    }
}

// --- FSM Logic ---
void wall_follower_fsm(uint16_t distance_cm, uint8_t is_forward_scan) {
    // IDLE: stop motors, enter sleep
    if (currentState == IDLE) {
        motor_stop();
        servo_set_angle(90);
        printf("Entering sleep mode. Send 's' to start.\n");
        _delay_ms(10);

        wdt_disable();
        set_sleep_mode(SLEEP_MODE_IDLE);
        sleep_enable();
        sleep_cpu();
        sleep_disable();
        wdt_enable(WDTO_2S);

        printf("Woke up!\n");
        return;
    }

    static uint8_t lost_wall = 0;

    // ==========================================================
    // FORWARD SCAN: Check for obstacles ahead
    // ==========================================================
    if (is_forward_scan && distance_cm > 0 && distance_cm < OBSTACLE_DISTANCE_CM) {
        // Obstacle detected — stop first
        motor_stop();
        wdt_reset();
        _delay_ms(200);

        // Pivot left until front is clear AND right wall is found
        while (1) {
            wdt_reset();

            // 1. Scan front
            servo_set_angle(90);
            _delay_ms(200);
            sensor_reset_measurement();
            sensor_trigger();
            _delay_ms(50);
            uint16_t front_dist = sensor_is_measurement_complete()
                ? (sensor_get_pulse_width() / 116) : 60;

            // 2. Scan right
            servo_set_angle(0);
            _delay_ms(200);
            sensor_reset_measurement();
            sensor_trigger();
            _delay_ms(50);
            uint16_t right_dist = sensor_is_measurement_complete()
                ? (sensor_get_pulse_width() / 116) : 60;

            printf("Avoid: F=%u R=%u\n", front_dist, right_dist);

            // 3. Front clear AND right wall detected?
            if (front_dist >= 35 && right_dist > 0 && right_dist <= 35) {
                break;  // Aligned — stop pivoting
            }

            // 4. Still blocked — pivot left
            motor_forward();
            motor_set_speeds(0, 70);   // Left stopped, right pushes
            _delay_ms(500);
            motor_stop();

            // Check if user sent stop
            if (currentState == IDLE) return;
        }

        // Return servo to right, clear the corner
        servo_set_angle(0);
        _delay_ms(200);
        wdt_reset();

        motor_forward();
        motor_set_speeds(MOTOR_SPEED, MOTOR_SPEED);
        _delay_ms(250);

        currentState = FORWARD;
        return;
    }

    // ==========================================================
    // RIGHT SCAN: Wall following + wall-lost detection
    // ==========================================================
    if (!is_forward_scan) {
        if (distance_cm > 0 && distance_cm <= 40) {
            // Wall visible
            lost_wall = 0;

            // Maintain target distance with ±2cm threshold
            if (distance_cm < (g_target_distance_cm - 2)) {
                currentState = TURN_LEFT;
            } else if (distance_cm > (g_target_distance_cm + 2)) {
                currentState = TURN_RIGHT;
            } else {
                currentState = FORWARD;
            }
        } else {
            // Wall lost (>40cm or bad reading)

            // Corner clearance: drive straight briefly when wall first disappears
            if (lost_wall == 0) {
                motor_forward();
                motor_set_speeds(MOTOR_SPEED, MOTOR_SPEED);
                _delay_ms(400);
                wdt_reset();
            }

            lost_wall = 1;
            currentState = TURN_RIGHT;
        }
    }

    // ==========================================================
    // STATE ACTION LOGIC - MOTOR ACTIONS
    // ==========================================================
    switch (currentState) {
        case FORWARD:
            motor_forward();
            motor_set_speeds(MOTOR_SPEED, MOTOR_SPEED);
            break;
        case TURN_LEFT:
            // Gentle arc away from wall
            motor_forward();
            motor_set_speeds(25, 35);
            break;
        case TURN_RIGHT:
            motor_forward();
            if (lost_wall) {
                // Sharp pivot right to wrap corner
                motor_set_speeds(35, 5);
            } else {
                // Gentle arc toward wall
                motor_set_speeds(35, 25);
            }
            break;
        case IDLE:
            motor_stop();
            break;
    }
}

int main(void) {
    // Initialize drivers
    uart_init();
    stdout = uart_get_stream();
    motor_init();
    sensor_init();
    servo_init();
    sei();

    printf("Robot Initialized!\n");
    printf("Send 's' to start, 'p' to pause.\n");
    printf("Send '+'/'-' to adjust target, 'w' to save.\n");

    settings_load();

    wdt_enable(WDTO_2S);
    printf("Watchdog enabled.\n");

    uint8_t look_forward = 1;

    while (1) {
        if (currentState != IDLE) {
            // Alternate servo between forward and right
            if (look_forward) {
                servo_set_angle(90);   // Front
            } else {
                servo_set_angle(0);    // Right (wall)
            }
            _delay_ms(150);

            // Trigger sensor
            if (!sensor_is_measurement_complete()) {
                sensor_trigger();
            }
            _delay_ms(50);

            // Process reading
            if (sensor_is_measurement_complete()) {
                uint16_t distance_cm = sensor_get_pulse_width() / 116;

                wall_follower_fsm(distance_cm, look_forward);

                if (look_forward) {
                    printf("FWD  -> State: %d, Dist: %u cm\n", currentState, distance_cm);
                } else {
                    printf("SIDE -> State: %d, Dist: %u cm\n", currentState, distance_cm);
                }

                look_forward = !look_forward;
                sensor_reset_measurement();
            }
        } else {
            // IDLE — trigger FSM to enter sleep
            wall_follower_fsm(0, 1);
        }

        wdt_reset();
    }

    return 0;
}