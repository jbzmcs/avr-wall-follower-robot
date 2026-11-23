// main.c - Autonomous Wall Follower with PD Steering
//
// Control strategy: the servo-mounted ultrasonic sensor lives pointed at the
// right wall and steers continuously with a PD controller; every 150ms it
// swings forward to check for obstacles. Distances are handled in inches.

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
#include "drivers/servo.h"
#include "drivers/time.h"
#include "drivers/uart.h"

// --- EEPROM Target Distance ---
#define CM_TO_IN              2.54
uint16_t g_target_distance_cm = 23;               // ~9.0 inches
uint16_t EEMEM g_target_distance_eeprom;
#define erasedMemory          0xFFFF
static double g_target_in = 23.0 / CM_TO_IN;      // control target in inches

// --- Distance Thresholds (inches) ---
#define OBSTACLE_TH_IN        14.0    // front obstacle -> avoid
#define WALL_LOST_TH_IN       18.0    // right wall too far -> seek
#define DISTANCE_TOLERANCE    2.5     // "well aligned" band
#define EMERGENCY_STOP_DIST   2.0     // right wall dangerously close

// --- Speed Settings (fraction of full power, scaled by MOTOR_PWM_TOP) ---
#define MOTOR_PWM_TOP         200     // PWM ceiling for 100% speed
#define CRUISE_SPEED          0.25
#define MAX_SPEED             0.32    // only when well aligned
#define MIN_SPEED             0.15
#define TURN_SPEED            0.22    // in-place pivots
#define BACKUP_SPEED          0.18    // emergency reverse

// --- PD Controller ---
#define KP                    0.035
#define KD                    0.015
#define MAX_STEERING          0.15

// --- Timing ---
#define SERVO_STABILIZE_MS    100
#define FRONT_CHECK_INTERVAL_MS 150
#define RIGHT_READ_DELAY_MS   20
#define TURN_DURATION_MS      200

// --- Measurement Validation / Filtering ---
#define FILTER_WINDOW         3
#define BAD_READS_FOR_LOST    3

// --- FSM States ---
typedef enum {
    STATE_IDLE,             // asleep, waiting for 's'
    STATE_STARTUP,          // waiting for a valid first reading
    STATE_RUNNING,          // continuous wall-following
    STATE_AVOID_OBSTACLE,   // front blocked, pivoting left
    STATE_FIND_WALL,        // right wall lost, curving right
    STATE_EMERGENCY_STOP    // too close, back off and recover
} RobotState;

static volatile RobotState currentState = STATE_IDLE;

static const char *state_name(RobotState state) {
    switch (state) {
        case STATE_IDLE:           return "IDLE";
        case STATE_STARTUP:        return "STARTUP";
        case STATE_RUNNING:        return "RUNNING";
        case STATE_AVOID_OBSTACLE: return "AVOID_OBSTACLE";
        case STATE_FIND_WALL:      return "FIND_WALL";
        case STATE_EMERGENCY_STOP: return "EMERGENCY_STOP";
        default:                   return "UNKNOWN";
    }
}

// --- Filter / Controller State ---
static double  right_history[FILTER_WINDOW] = {0};
static uint8_t history_index = 0;
static double  previous_error = 0.0;
static uint8_t bad_reading_count = 0;
static volatile uint8_t idle_message_printed = 0;

static void control_reset(void) {
    uint8_t slot;

    for (slot = 0; slot < FILTER_WINDOW; slot++) {
        right_history[slot] = 0.0;
    }
    history_index = 0;
    previous_error = 0.0;
    bad_reading_count = 0;
}

// --- Motor Helpers (fractional speed -> PWM counts) ---
static uint8_t pct_to_pwm(double pct) {
    if (pct < 0.0) pct = 0.0;
    if (pct > 1.0) pct = 1.0;
    return (uint8_t)(pct * MOTOR_PWM_TOP);
}

static void drive_forward_pct(double left_pct, double right_pct) {
    motor_forward();
    motor_set_speeds(pct_to_pwm(left_pct), pct_to_pwm(right_pct));
}

static void pivot_left_pct(double pct) {
    motor_turn_left();
    motor_set_speeds(pct_to_pwm(pct), pct_to_pwm(pct));
}

static void backup_pct(double pct) {
    motor_backward();
    motor_set_speeds(pct_to_pwm(pct), pct_to_pwm(pct));
}

// --- Measurements ---
// Two-sample reading for the wall side: average both when valid.
static double take_measurement(void) {
    double d1 = sensor_read_inches();
    _delay_ms(10);
    double d2 = sensor_read_inches();

    if (d1 > 0.0 && d2 > 0.0) {
        return (d1 + d2) / 2.0;
    } else if (d1 > 0.0) {
        return d1;
    } else if (d2 > 0.0) {
        return d2;
    }
    return 0.0;
}

// Single quick reading for the front check.
static double take_quick_measurement(void) {
    return sensor_read_inches();
}

// Moving average over the last valid side readings.
// A bad reading (0.0) returns the last average instead of polluting history.
static double apply_filter(double new_value) {
    double sum = 0.0;
    uint8_t slot;
    uint8_t count = 0;

    if (new_value == 0.0) {
        for (slot = 0; slot < FILTER_WINDOW; slot++) {
            if (right_history[slot] > 0.0) {
                sum += right_history[slot];
                count++;
            }
        }
        return (count > 0) ? (sum / count) : g_target_in;
    }

    right_history[history_index] = new_value;
    history_index = (history_index + 1) % FILTER_WINDOW;

    for (slot = 0; slot < FILTER_WINDOW; slot++) {
        if (right_history[slot] > 0.0) {
            sum += right_history[slot];
            count++;
        }
    }

    return (count > 0) ? (sum / count) : new_value;
}

// --- PD Steering ---
// Positive steering = veer right (toward wall), negative = veer left (away).
static double calculate_steering(double right_distance) {
    double error = right_distance - g_target_in;
    double derivative = error - previous_error;
    double steering = (KP * error) + (KD * derivative);

    previous_error = error;

    if (steering > MAX_STEERING) steering = MAX_STEERING;
    if (steering < -MAX_STEERING) steering = -MAX_STEERING;

    return steering;
}

// Steering shifts the differential: left = base + steer, right = base - steer.
static void update_continuous_drive(double steering, double base_speed) {
    double left_speed = base_speed + steering;
    double right_speed = base_speed - steering;

    if (left_speed < MIN_SPEED) left_speed = MIN_SPEED;
    if (right_speed < MIN_SPEED) right_speed = MIN_SPEED;
    if (left_speed > MAX_SPEED) left_speed = MAX_SPEED;
    if (right_speed > MAX_SPEED) right_speed = MAX_SPEED;

    drive_forward_pct(left_speed, right_speed);
}

// --- EEPROM Settings ---
void settings_load(void) {
    uint16_t stored = eeprom_read_word(&g_target_distance_eeprom);

    if (stored == erasedMemory) {
        eeprom_update_word(&g_target_distance_eeprom, g_target_distance_cm);
        printf("EEPROM empty, default target written: %u cm\n", g_target_distance_cm);
    } else {
        g_target_distance_cm = stored;
        printf("EEPROM loaded target: %u cm\n", g_target_distance_cm);
    }
    g_target_in = (double)g_target_distance_cm / CM_TO_IN;
    printf("Control target: %.2f in\n", g_target_in);
}

// --- UART Command Interface ---
ISR(USART_RX_vect) {
    unsigned char command = UDR0;

    switch (command) {
        case 's':
            servo_wake();
            currentState = STATE_STARTUP;
            control_reset();
            idle_message_printed = 0;
            printf("\nCOMMAND: START -> FSM=%s target=%u cm\n",
                   state_name(currentState), g_target_distance_cm);
            break;

        case 'p':
            currentState = STATE_IDLE;
            motor_stop();
            idle_message_printed = 0;
            printf("\nCOMMAND: STOP -> FSM=%s\n", state_name(currentState));
            break;

        case '+':
            g_target_distance_cm++;
            g_target_in = (double)g_target_distance_cm / CM_TO_IN;
            printf("Target changed: %u cm (send 'w' to save)\n", g_target_distance_cm);
            break;

        case '-':
            if (g_target_distance_cm > 1) {
                g_target_distance_cm--;
            }
            g_target_in = (double)g_target_distance_cm / CM_TO_IN;
            printf("Target changed: %u cm (send 'w' to save)\n", g_target_distance_cm);
            break;

        case 'w':
            eeprom_update_word(&g_target_distance_eeprom, g_target_distance_cm);
            printf("EEPROM saved target: %u cm\n", g_target_distance_cm);
            break;
    }
}

static void report_transition(RobotState before, RobotState after) {
    if (before != after) {
        printf("FSM: %s -> %s\n", state_name(before), state_name(after));
    }
}

int main(void) {
    double right_distance;
    double front_distance = 0.0;
    uint32_t last_front_check = 0;
    RobotState before_state;

    uart_init();
    stdout = uart_get_stream();
    motor_init();
    sensor_init();
    servo_init();
    time_init();
    sei();

    printf("\n=== AVR WALL FOLLOWER ===\n");
    printf("Continuous PD wall-following, servo front scan every %d ms\n",
           FRONT_CHECK_INTERVAL_MS);
    printf("UART commands: s=start, p=stop, +=target up, -=target down, w=write EEPROM\n");

    settings_load();
    right_distance = g_target_in;

    wdt_enable(WDTO_2S);
    printf("Watchdog enabled: 2 second reset window\n");
    printf("Boot complete. Initial FSM=%s. Send 's' to start.\n", state_name(currentState));

    for (;;) {
        before_state = currentState;

        switch (currentState) {
            case STATE_IDLE:
                motor_stop();
                servo_move(SERVO_PULSE_FRONT);
                _delay_ms(SERVO_STABILIZE_MS);
                servo_sleep();
                if (!idle_message_printed) {
                    printf("Entering sleep mode. Send 's' to start.\n");
                    idle_message_printed = 1;
                }
                _delay_ms(10);

                wdt_disable();
                time_pause();
                set_sleep_mode(SLEEP_MODE_IDLE);
                sleep_enable();
                sleep_cpu();
                sleep_disable();
                time_resume();
                wdt_enable(WDTO_2S);

                printf("Woke up!\n");
                break;

            case STATE_STARTUP:
                // Get one valid side reading before moving.
                servo_move(SERVO_PULSE_RIGHT);
                _delay_ms(SERVO_STABILIZE_MS);
                right_distance = apply_filter(take_measurement());

                printf("Startup R: %.2f in\n", right_distance);

                if (right_distance > 0.0) {
                    printf("Initial reading OK, starting...\n");
                    currentState = STATE_RUNNING;
                    last_front_check = millis();
                    drive_forward_pct(CRUISE_SPEED, CRUISE_SPEED);
                } else {
                    printf("Waiting for valid reading...\n");
                    _delay_ms(100);
                }
                break;

            case STATE_RUNNING: {
                double raw;

                // Servo lives at the right wall during normal running.
                if (!servo_at_position(SERVO_PULSE_RIGHT)) {
                    servo_move(SERVO_PULSE_RIGHT);
                    _delay_ms(SERVO_STABILIZE_MS);
                }

                raw = take_measurement();
                right_distance = apply_filter(raw);

                printf("R: %.2f in\n", right_distance);

                if (raw == 0.0) {
                    bad_reading_count++;
                    printf("Bad right reading!\n");
                    if (bad_reading_count > BAD_READS_FOR_LOST) {
                        printf("Wall lost - too many bad readings!\n");
                        currentState = STATE_FIND_WALL;
                        break;
                    }
                } else {
                    bad_reading_count = 0;
                }

                if (right_distance > 0.0 && right_distance < EMERGENCY_STOP_DIST) {
                    printf("EMERGENCY: Too close to wall!\n");
                    currentState = STATE_EMERGENCY_STOP;
                    break;
                }

                if (right_distance > WALL_LOST_TH_IN) {
                    printf("Wall too far, seeking...\n");
                    currentState = STATE_FIND_WALL;
                    break;
                }

                // Periodic front obstacle check.
                if ((millis() - last_front_check) > FRONT_CHECK_INTERVAL_MS) {
                    drive_forward_pct(MIN_SPEED, MIN_SPEED);   // slow down while scanning

                    servo_move(SERVO_PULSE_FRONT);
                    _delay_ms(SERVO_STABILIZE_MS);
                    front_distance = take_quick_measurement();
                    printf("F: %.2f in\n", front_distance);

                    servo_move(SERVO_PULSE_RIGHT);
                    _delay_ms(SERVO_STABILIZE_MS);
                    wdt_reset();

                    last_front_check = millis();

                    if (front_distance > 0.0 && front_distance < OBSTACLE_TH_IN) {
                        printf("OBSTACLE at %.2f in!\n", front_distance);
                        currentState = STATE_AVOID_OBSTACLE;
                        break;
                    }
                }

                // PD steering with alignment-based speed.
                {
                    double steering = calculate_steering(right_distance);
                    double speed = CRUISE_SPEED;
                    double error_mag = right_distance - g_target_in;

                    if (error_mag < 0) error_mag = -error_mag;

                    if (error_mag < DISTANCE_TOLERANCE) {
                        speed = MAX_SPEED;      // well aligned: a bit faster
                    } else if (error_mag > 3.0) {
                        speed = MIN_SPEED;      // way off: crawl
                    }

                    update_continuous_drive(steering, speed);
                }

                _delay_ms(RIGHT_READ_DELAY_MS);
                break;
            }

            case STATE_AVOID_OBSTACLE:
                motor_stop();
                _delay_ms(100);

                printf("Avoiding obstacle - turning left\n");

                pivot_left_pct(TURN_SPEED);
                _delay_ms(TURN_DURATION_MS);
                motor_stop();
                _delay_ms(50);
                wdt_reset();

                // Re-check front after the turn.
                servo_move(SERVO_PULSE_FRONT);
                _delay_ms(SERVO_STABILIZE_MS);
                front_distance = take_quick_measurement();
                printf("After turn F: %.2f in\n", front_distance);

                servo_move(SERVO_PULSE_RIGHT);
                _delay_ms(SERVO_STABILIZE_MS);

                if (front_distance > 0.0 && front_distance < OBSTACLE_TH_IN) {
                    printf("Still blocked, turning more...\n");
                    break;   // stay in this state, pivot again
                }

                // Clear: creep forward and resume following.
                drive_forward_pct(CRUISE_SPEED, CRUISE_SPEED);
                _delay_ms(150);

                last_front_check = millis();
                printf("Resuming wall follow\n");
                currentState = STATE_RUNNING;
                break;

            case STATE_FIND_WALL:
                if (!servo_at_position(SERVO_PULSE_RIGHT)) {
                    servo_move(SERVO_PULSE_RIGHT);
                    _delay_ms(SERVO_STABILIZE_MS);
                }

                printf("Seeking wall - curving right\n");

                // Curve right: left wheel faster than right.
                drive_forward_pct(CRUISE_SPEED, MIN_SPEED);
                _delay_ms(200);

                right_distance = apply_filter(take_measurement());
                printf("Seeking R: %.2f in\n", right_distance);

                if (right_distance > 0.0 && right_distance < WALL_LOST_TH_IN) {
                    printf("Wall found!\n");
                    bad_reading_count = 0;
                    last_front_check = millis();
                    currentState = STATE_RUNNING;
                }
                // Otherwise keep curving right next iteration.
                break;

            case STATE_EMERGENCY_STOP:
                motor_stop();
                printf("EMERGENCY - backing up\n");
                _delay_ms(100);

                backup_pct(BACKUP_SPEED);
                _delay_ms(200);
                motor_stop();
                _delay_ms(50);
                wdt_reset();

                printf("Turning away from wall\n");
                pivot_left_pct(TURN_SPEED);
                _delay_ms(150);
                motor_stop();
                _delay_ms(50);

                right_distance = apply_filter(take_measurement());
                printf("After correction R: %.2f in\n", right_distance);

                last_front_check = millis();
                currentState = STATE_RUNNING;
                break;

            default:
                currentState = STATE_STARTUP;
                break;
        }

        report_transition(before_state, currentState);

        _delay_ms(10);
        wdt_reset();
    }

    return 0;
}
