#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "gpio.h"   
#include "sensor.h"

// Define Trig pin connection (PB4 / Arduino D12)
#define TRIG_PORT   GPIO_PORT_B
#define TRIG_PIN    4    

// Define Echo pin connection (PB5 / Arduino D13)
#define ECHO_PORT   GPIO_PORT_B
#define ECHO_PIN    5

// Global variables for ISR communication
static volatile uint16_t pulse_width = 0;
static volatile uint8_t measurement_complete = 0;


ISR(PCINT0_vect) {
    static uint16_t start_time = 0;

    // Read the current state of the Echo pin (PB5)
    // If it is HIGH, we just experienced a RISING edge (Start of pulse)
    if (gpio_read(ECHO_PORT, ECHO_PIN) == GPIO_PIN_HIGH) {
        start_time = TCNT1; // Manually save the current timer value
    } 
    // If it is LOW, we just experienced a FALLING edge (End of pulse)
    else {
        uint16_t end_time = TCNT1;
        
        // Calculate duration (handles unsigned overflow naturally)
        pulse_width = end_time - start_time;
        
        // Set flag to notify main loop
        measurement_complete = 1;
    }
}

void sensor_init(void) {
    // Trig (PB4) as Output
    gpio_set_direction(TRIG_PORT, TRIG_PIN, GPIO_PIN_OUTPUT);
    
    // Echo (PB5) as Input
    gpio_set_direction(ECHO_PORT, ECHO_PIN, GPIO_PIN_INPUT);
    
    // Setup Timer 1
    // Set Prescaler to 8 (CS11 bit). System clock 16MHz / 8 = 2MHz tick (0.5us).
    TCCR1B |= (1 << CS11); 
    
    // Configure Pin Change Interrupts for PB5
    // 1. Enable Pin Change Interrupts for PORTB group (PCIE0)
    PCICR |= (1 << PCIE0);
    
    // 2. Enable the specific mask for PB5 (PCINT5) so only this pin triggers the interrupt
    PCMSK0 |= (1 << PCINT5);
    
    // Ensure Trig is low initially
    gpio_write(TRIG_PORT, TRIG_PIN, GPIO_PIN_LOW);
}

void sensor_trigger(void) {
    // Clear the completion flag to start a new reading cycle
    measurement_complete = 0;
    
    // Reset timer to prevent overflowing too soon during our measurement
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

void sensor_reset_measurement(void) {
    measurement_complete = 0;
}