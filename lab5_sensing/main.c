//  Lab 5 Application: Ultrasonic Sensing

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "gpio.h"
#include "sensor.h"

int main(void) {
    // Use a debug LED on PD5
    gpio_set_direction(GPIO_PORT_D, 5, GPIO_PIN_OUTPUT);
    
    // Initialize sensor driver and Timer1
    sensor_init();
    
    // Enable global interrupts
    sei(); 

    while (1) {
        // Only start a new measurement if the previous one is processed/done.
        if (!sensor_is_measurement_complete()) {
            sensor_trigger();
        }

        // 60ms delay ensures it won't trigger too fast 
        _delay_ms(60); 

        if (sensor_is_measurement_complete()) {
            // Retrieve raw timer ticks (0.5us per tick)
            uint16_t pw = sensor_get_pulse_width();
            
            // ACTIVITY 5.2: Process the Raw Data
            // Formula: Distance (cm) = (pulse_width * 0.5us * 34300 cm/s) / 2
            // Simplification: Distance (cm) ~= pulse_width / 116
            uint16_t distance_cm = pw / 116; 
            
            // Blink the LED if valid reading
            // Only blink if distance < 20cm (wall detected)
            if (distance_cm > 0 && distance_cm < 400) {
                gpio_write(GPIO_PORT_D, 5, GPIO_PIN_HIGH);
                _delay_ms(50);
                gpio_write(GPIO_PORT_D, 5, GPIO_PIN_LOW);
            }
        }
    }
    return 0;
}