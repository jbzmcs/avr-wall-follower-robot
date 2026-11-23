// main.c - Refactored with GPIO Driver
#define F_CPU 16000000UL
#include <util/delay.h>
#include "gpio.h"

// --- Pin Definitions for Clarity ---
#define LED_PORT GPIO_PORT_D
#define LED_PIN  2

#define M_BUTTON_PORT GPIO_PORT_B
#define M_BUTTON_PIN  3
#define S_BUTTON_PORT GPIO_PORT_B   // Added S button definition
#define S_BUTTON_PIN  4
#define U_BUTTON_PORT GPIO_PORT_B   // Added U button definition
#define U_BUTTON_PIN  5

#define DOT_LENGTH_MS 250

void dot() {
    gpio_write(LED_PORT, LED_PIN, GPIO_PIN_HIGH);
    _delay_ms(DOT_LENGTH_MS);
    gpio_write(LED_PORT, LED_PIN, GPIO_PIN_LOW);
    _delay_ms(DOT_LENGTH_MS);
}

// Rewrite the dash() function to use gpio_write()
void dash() {
    gpio_write(LED_PORT, LED_PIN, GPIO_PIN_HIGH);
    _delay_ms(DOT_LENGTH_MS * 3);
    gpio_write(LED_PORT, LED_PIN, GPIO_PIN_LOW);
    _delay_ms(DOT_LENGTH_MS);
}

int main(void) {
    // --- PIN CONFIGURATION using the driver ---
    gpio_set_direction(LED_PORT, LED_PIN, GPIO_PIN_OUTPUT);

    gpio_set_direction(M_BUTTON_PORT, M_BUTTON_PIN, GPIO_PIN_INPUT);  // Configured M
    gpio_set_pullup(M_BUTTON_PORT, M_BUTTON_PIN, 1);

    gpio_set_direction(S_BUTTON_PORT, S_BUTTON_PIN, GPIO_PIN_INPUT);  // Configure S
    gpio_set_pullup(S_BUTTON_PORT, S_BUTTON_PIN, 1);

    gpio_set_direction(U_BUTTON_PORT, U_BUTTON_PIN, GPIO_PIN_INPUT);  // Configure U
    gpio_set_pullup(U_BUTTON_PORT, U_BUTTON_PIN, 1);
    
    // --- MAIN LOOP using the driver ---
    while (1) {
        // Call dash(), dash() for 'M'
        // Add checks for the 'S' and 'U' buttons using gpio_read()
        
        // M = --
        if (gpio_read(M_BUTTON_PORT, M_BUTTON_PIN) == GPIO_PIN_LOW) {
            dash(); dash();
            _delay_ms(DOT_LENGTH_MS * 2);
        }
         // S = ...
        if (gpio_read(S_BUTTON_PORT, S_BUTTON_PIN) == GPIO_PIN_LOW) {
            dot(); dot(); dot();
            _delay_ms(DOT_LENGTH_MS * 2);
        }
        // U = ..-
        if (gpio_read(U_BUTTON_PORT, U_BUTTON_PIN) == GPIO_PIN_LOW) {
            dot(); dot(); dash();
            _delay_ms(DOT_LENGTH_MS * 2);
        }
    }
}