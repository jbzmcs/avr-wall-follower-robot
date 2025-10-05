// Activity 3.1 Configuring an External Interrupt
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include "gpio.h"
// --- Global Volatile Flag ---
// This flag is our communication channel between the ISR and main loop.
volatile uint8_t g_button_pressed_flag = 0;

// --- NEW: ISR for External Interrupt 0 (INT0) ---
ISR(INT0_vect) {
    // Set the flag. That's it. Keep it fast!
    g_button_pressed_flag = 1;
}

// --- NEW: Initialization for External Interrupt 0 ---
void external_interrupt_init(void) {
    // Use our GPIO driver to configure the button pin.
    gpio_set_direction(GPIO_PORT_D, 2, GPIO_PIN_INPUT);
    gpio_set_pullup(GPIO_PORT_D, 2, 1);

    // Configure EICRA to trigger INT0 on a falling edge.
    // You need to set ISC01 and clear ISC00.
    EICRA |= (1 << ISC01);
    EICRA &= ~(1 << ISC00);
    
    // Enable the external interrupt for INT0 in the EIMSK register.
    EIMSK |= (1 << INT0);
}