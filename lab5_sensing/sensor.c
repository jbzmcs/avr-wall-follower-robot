// For Activity 5.1 Building the Sensor Driver

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "gpio.h"   
#include "sensor.h"

// Define Trig pin connection (PD7) 
#define TRIG_PORT   GPIO_PORT_D
#define TRIG_PIN    7

// Global variables for ISR communication
volatile uint16_t pulse_width = 0;
volatile uint8_t measurement_complete = 0;

/* * ISR(TIMER1_CAPT_vect)
 * Handles the Input Capture event.
 * Implements the state machine to measure pulse duration.
 */
ISR(TIMER1_CAPT_vect) {
    // Static variable to track which edge we are waiting for
    // 0 = Waiting for Rising Edge (Start of pulse)
    // 1 = Waiting for Falling Edge (End of pulse)
    static uint8_t capture_state = 0; 
    static uint16_t start_time = 0;

    if (capture_state == 0) {
        // Caught the RISING edge (Start of Echo)
        start_time = ICR1; // Save the timer value at the moment of the edge

        // Reconfigure ICU to trigger on FALLING edge next
        // Clear ICES1 bit in TCCR1B
        TCCR1B &= ~(1 << ICES1);
        
        // Update state to wait for falling edge
        capture_state = 1;
    } 
    else {
        // Caught the FALLING edge (End of Echo)
        uint16_t end_time = ICR1;
        
        // Calculate duration
        // handles unsigned overflow naturally if end_time < start_time
        pulse_width = end_time - start_time;
        
        // Set flag to notify main loop
        measurement_complete = 1;

        // Reconfigure ICU to trigger on RISING edge for the next measurement
        // Set ICES1 bit in TCCR1B
        TCCR1B |= (1 << ICES1);
        
        // Reset state
        capture_state = 0;
    }
}

void sensor_init(void) {
    // Trig (PD7) as Output
    gpio_set_direction(TRIG_PORT, TRIG_PIN, GPIO_PIN_OUTPUT);
    // Echo (PB0) as Input (This corresponds to ICP1 pin)
    gpio_set_direction(GPIO_PORT_B, 0, GPIO_PIN_INPUT);
    
    // Set Prescaler to 8 (CS11 bit)
    // System clock is likely 16MHz. 16MHz / 8 = 2MHz tick (0.5us).
    // TCCR1B Register: Input Capture Edge Select (ICES1) | Prescaler (CS11)
    TCCR1B |= (1 << CS11); 
    
    // Configure ICU to trigger on RISING edge initially
    // Set ICES1 bit
    TCCR1B |= (1 << ICES1);

    // Enable Input Capture Interrupt (ICIE1) in TIMSK1
    TIMSK1 |= (1 << ICIE1);
    
    // Ensure Trig is low initially
    gpio_write(TRIG_PORT, TRIG_PIN, GPIO_PIN_LOW);
}

void sensor_trigger(void) {
    // Clear the completion flag to start a new reading cycle
    measurement_complete = 0;
    TCNT1 = 0;

    // Generate a 10us HIGH pulse on Trig pin
    gpio_write(TRIG_PORT, TRIG_PIN, GPIO_PIN_HIGH);
    _delay_us(10);
    gpio_write(TRIG_PORT, TRIG_PIN, GPIO_PIN_LOW);
}

uint8_t sensor_is_measurement_complete(void) {
    return measurement_complete;
}

uint16_t sensor_get_pulse_width(void) {
    return pulse_width;
}