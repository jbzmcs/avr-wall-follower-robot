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

// Default target distance
uint16_t g_target_distance_cm = 15;

// Use the EEMEM attribute to tell the compiler to reserve space in the EEPROM.
uint16_t EEMEM g_target_distance_eeprom;

// --- FSM and Robot Parameters ---
typedef enum { IDLE, FORWARD, TURN_LEFT, TURN_RIGHT } RobotState;
#define TARGET_DISTANCE_CM    g_target_distance_cm
#define DISTANCE_THRESHOLD_CM 5
#define MOTOR_SPEED           100
#define erasedMemory          0xFFFF

// Volatile global — shared between ISR and main loop
volatile RobotState currentState = IDLE;

// Differential speed for gentle arc turns
// #define TURN_FAST  70
// #define TURN_SLOW  50

#define TURN_LEFT_FAST   75
#define TURN_LEFT_SLOW   30

#define TURN_RIGHT_FAST  80    // Lower to slow down the right turn
#define TURN_RIGHT_SLOW  30

void settings_load(void){
    // Read the word from g_target_distance_eeprom.
    // If the read value is 0xFFFF (erased memory), it's the first run.
    // In that case, write the default value from g_target_distance_cm to the EEPROM
    // using eeprom_update_word(). Otherwise, update g_target_distance_cm
    // with the value you read from the EEPROM.

    uint16_t stored = eeprom_read_word(&g_target_distance_eeprom);
    
    if (stored == erasedMemory){
        // First run -- EEPROM is erased, write default
        eeprom_update_word(&g_target_distance_eeprom, g_target_distance_cm);
    } else {
        g_target_distance_cm = stored;
    }
    printf("Target distance set to %u cm\n", g_target_distance_cm);
}



// --- UART Receive Complete ISR  ---
ISR(USART_RX_vect) {
    unsigned char command = UDR0;
    switch (command){
    case 's':
        currentState = FORWARD;
        motor_forward();
        printf("COMMAND: Start\n");
        break;
    case 'p':
        currentState = IDLE;
        printf("COMMAND: Stop\n");
        break;
    case '+':
        g_target_distance_cm++;
        printf("Target: %u cm\n", g_target_distance_cm);
        break;
    case '-':
        if(g_target_distance_cm > 1){
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

// FSM logic
void wall_follower_fsm(uint16_t distance_cm) {

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
                // watchdog timer test loop
                // while(1); 
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
        case IDLE:
            motor_stop();
            printf("Entering sleep mode. Send 's' to start.\n");
            _delay_ms(10);

            wdt_disable();
            set_sleep_mode(SLEEP_MODE_IDLE);
            sleep_enable();
            sleep_cpu();
            sleep_disable();
            wdt_enable(WDTO_1S);

            printf("Woke up!\n");

            break;
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
    // Initialize UART first for debug output
    uart_init();
    stdout = uart_get_stream(); // Redirect stdout to our UART stream

    // Initialize all drivers
    motor_init(); 
    sensor_init();
    sei();

    printf("Robot Initialized!\n");
    printf("Send 's' to start, 'p' to pause.\n");
    printf("Send '+'/'-' to adjust target, 'w' to save.\n");

    
    // Load persistent settings from EEPROM 
    settings_load();

    // Enable the watchdog timer with a 1-second timeout.
    wdt_enable(WDTO_1S);
    printf("Watchdog enabled.\n");

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
                printf("State: %d, Distance: %u cm\n", currentState, distance_cm);
                wall_follower_fsm(distance_cm);
            }
            // Reset flag for next measurement
            sensor_reset_measurement();
        }
        wdt_reset();
    }
}