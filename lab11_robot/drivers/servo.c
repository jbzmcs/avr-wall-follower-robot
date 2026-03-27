#include <avr/io.h>
#include "servo.h"

#define SERVO_DDR   DDRB
#define SERVO_PIN   3

void servo_init(void) {
    // PB3 as output
    SERVO_DDR |= (1 << SERVO_PIN);

    // Fast PWM Mode 3, non-inverting on OC2A
    TCCR2A = (1 << COM2A1) | (1 << WGM21) | (1 << WGM20);

    // Prescaler 1024
    TCCR2B = (1 << CS22) | (1 << CS21) | (1 << CS20);

    // Start facing forward
    servo_set_angle(90);
}

void servo_set_angle(uint8_t angle) {
    if (angle > 180) angle = 180;

    // Map 0-180 degrees to OCR2A 8-38
    uint8_t pulse_ticks = 8 + ((uint16_t)angle * 30) / 180;

    OCR2A = pulse_ticks;
}