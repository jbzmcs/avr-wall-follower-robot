// drivers/servo.c - Scan servo on PB2 (OC1B) using Timer1 Fast PWM
#include <avr/io.h>
#include "gpio.h"
#include "servo.h"

#define SERVO_PORT  GPIO_PORT_B
#define SERVO_PIN   2

// Track where the horn is so redundant moves can be skipped.
static volatile int16_t current_position = SERVO_PULSE_FRONT;

void servo_init(void) {
    gpio_set_direction(SERVO_PORT, SERVO_PIN, GPIO_PIN_OUTPUT);

    // Fast PWM mode 15 (TOP = OCR1A), non-inverting on OC1B
    TCCR1A = (1 << WGM11) | (1 << WGM10) | (1 << COM1B1);
    TCCR1B = (1 << WGM13) | (1 << WGM12);

    // Prescaler 64: 16MHz / 64 = 250kHz -> 4us per count
    TCCR1B |= (1 << CS11) | (1 << CS10);

    // 20ms servo frame: 250kHz * 20ms = 5000 counts
    OCR1A = 4999;

    // Boot facing forward
    OCR1B = SERVO_PULSE_FRONT;
    current_position = SERVO_PULSE_FRONT;
}

uint8_t servo_at_position(int16_t pulse) {
    int16_t diff = current_position - pulse;
    if (diff < 0) diff = -diff;
    return (diff <= 5);
}

uint8_t servo_move(int16_t pulse) {
    // Clamp to the mechanical range
    if (pulse < SERVO_PULSE_RIGHT) pulse = SERVO_PULSE_RIGHT;
    if (pulse > SERVO_PULSE_LEFT)  pulse = SERVO_PULSE_LEFT;

    if (servo_at_position(pulse)) {
        return 0;
    }

    OCR1B = pulse;
    current_position = pulse;
    return 1;
}

void servo_sleep(void) {
    // Detach OC1B so the servo stops receiving hold pulses while idle.
    TCCR1A &= ~(1 << COM1B1);
    gpio_write(SERVO_PORT, SERVO_PIN, GPIO_PIN_LOW);
}

void servo_wake(void) {
    TCCR1A |= (1 << COM1B1);
}
