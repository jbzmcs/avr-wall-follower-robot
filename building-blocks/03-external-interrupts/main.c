#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include "gpio.h"

/* System tick (from 02-cooperative-multitasking) */
volatile uint32_t system_ticks = 0;

ISR(TIMER1_COMPA_vect) {
    system_ticks++;
}
void system_tick_init(void) {
    
    cli();                  // Disable global interrupts

    TCCR1A = 0;             // Normal port operation, WGM bits in TCCR18
    TCCR1B = 0;

    // Configure Timer1 in CTC mode (WGM12 bit).
    TCCR1B |= (1 << WGM12); // set CTC mode: WGM12 = 1 (mode 4)

    // Set the compare value in OCR1A to 249.
    OCR1A = 249;
    // Set the prescaler to 64 (CS11 and CS10 bits).
    TCCR1B |= (1 << CS11) | (1 << CS10);

    // Enable the Timer1 Compare A interrupt (OCIE1A bit).
    TIMSK1 |= (1 << OCIE1A);

    sei(); // Enable global interrupts. This must be called for any interrupt to work.
}

/* --- External interrupt (INT0) --- */

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

/* --- Debouncing and state management --- */

#define STATUS_LED_PORT GPIO_PORT_D
#define STATUS_LED_PIN 5
#define USER_BUTTON_PORT GPIO_PORT_D
#define USER_BUTTON_PIN 2

volatile uint8_t g_led_is_blinking = 1;

void task_process_button_press(void) {
    static uint32_t last_debounce_time = 0;
    const uint16_t DEBOUNCE_DELAY_MS = 50;
    
    // Check if the ISR has set our flag AND we are outside the debounce window
    if (g_button_pressed_flag && (system_ticks - last_debounce_time) > DEBOUNCE_DELAY_MS) {
        g_led_is_blinking = !g_led_is_blinking; // Toggle the application state
        g_button_pressed_flag = 0; // Clear the flag now that we've handled it
        last_debounce_time = system_ticks; // Start the debounce timer
    }
}

void task_blink_led(void) {
    static uint32_t last_toggle_time = 0;
    if (!g_led_is_blinking) {
        gpio_write(STATUS_LED_PORT, STATUS_LED_PIN, GPIO_PIN_LOW);
        return;
    }
    if ((system_ticks - last_toggle_time) >= 500) {
        // Toggle the LED using your GPIO driver
        uint8_t cur = gpio_read(STATUS_LED_PORT, STATUS_LED_PIN);
        gpio_write(STATUS_LED_PORT, STATUS_LED_PIN,
            cur == GPIO_PIN_HIGH ? GPIO_PIN_LOW : GPIO_PIN_HIGH);
        last_toggle_time = system_ticks;
    }
}

int main(void) {
    // --- INITIALIZATION ---
    gpio_set_direction(STATUS_LED_PORT, STATUS_LED_PIN, GPIO_PIN_OUTPUT);
    external_interrupt_init(); // Sets up button pin as well
    system_tick_init();
    
    sei(); // Enable global interrupts

    // --- SUPERLOOP ---
    while (1) {
        task_process_button_press(); // Check for and handle events
        task_blink_led(); // Update state based on events
    }
    return 0;
}