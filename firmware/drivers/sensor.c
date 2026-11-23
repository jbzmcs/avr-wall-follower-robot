// drivers/sensor.c - HC-SR04 echo timing via PCINT0 + micros() timestamps
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "gpio.h"
#include "sensor.h"
#include "time.h"

// Trig on PB4 (Arduino D12), Echo on PB5 (Arduino D13)
#define TRIG_PORT   GPIO_PORT_B
#define TRIG_PIN    4
#define ECHO_PORT   GPIO_PORT_B
#define ECHO_PIN    5

// Round-trip us -> one-way inches: (0.0343 cm/us / 2) / 2.54
#define US_TO_INCHES        0.00675197

#define ECHO_TIMEOUT_US     15000UL   // ~255in max, fast failure detection
#define MIN_VALID_IN        1.5
#define MAX_VALID_IN        80.0

static volatile uint32_t echo_start_us = 0;
static volatile uint32_t echo_end_us = 0;
static volatile uint8_t  echo_ready = 0;
static volatile uint8_t  echo_started = 0;

ISR(PCINT0_vect) {
    uint32_t now = micros();   // timestamp first, minimal latency

    if (gpio_read(ECHO_PORT, ECHO_PIN) == GPIO_PIN_HIGH) {
        // Rising edge: echo pulse begins
        echo_start_us = now;
        echo_started = 1;
    } else if (echo_started) {
        // Falling edge: echo pulse ends (only valid after a rising edge)
        echo_end_us = now;
        echo_ready = 1;
        echo_started = 0;
    }
}

void sensor_init(void) {
    gpio_set_direction(TRIG_PORT, TRIG_PIN, GPIO_PIN_OUTPUT);
    gpio_set_direction(ECHO_PORT, ECHO_PIN, GPIO_PIN_INPUT);
    gpio_write(TRIG_PORT, TRIG_PIN, GPIO_PIN_LOW);

    // Pin change interrupt on PB5 only
    PCICR  |= (1 << PCIE0);
    PCMSK0 |= (1 << PCINT5);
}

double sensor_read_inches(void) {
    uint32_t start;
    uint32_t duration_us;
    double distance_in;

    echo_ready = 0;
    echo_started = 0;

    // 10us trigger pulse
    gpio_write(TRIG_PORT, TRIG_PIN, GPIO_PIN_HIGH);
    _delay_us(10);
    gpio_write(TRIG_PORT, TRIG_PIN, GPIO_PIN_LOW);

    start = micros();
    while (!echo_ready) {
        if ((micros() - start) > ECHO_TIMEOUT_US) {
            return 0.0;   // timeout / nothing in range
        }
    }

    duration_us = echo_end_us - echo_start_us;
    distance_in = duration_us * US_TO_INCHES;

    if (distance_in < MIN_VALID_IN || distance_in > MAX_VALID_IN) {
        return 0.0;
    }
    return distance_in;
}
