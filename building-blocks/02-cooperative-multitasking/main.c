// main.c - Cooperative multitasking on a 1 ms Timer1 system tick
#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include "gpio.h"

// 'volatile' is critical. It tells the compiler that this variable can change
// at any time (by the ISR), so it must always be read from memory.
volatile uint32_t system_ticks = 0;

// This is the Interrupt Service Routine for Timer1 Compare Match A.
// This function is automatically called by the hardware every 1ms.
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
// --- Non-blocking tasks ---
#define STATUS_LED_PORT GPIO_PORT_D
#define STATUS_LED_PIN 5
#define USER_BUTTON_PORT GPIO_PORT_D
#define USER_BUTTON_PIN 2

void task_blink_status_led(void) {
    static uint32_t last_toggle_time = 0;
    if ((system_ticks - last_toggle_time) >= 500) { // 500ms interval
        // Toggle the LED using your gpio_write/gpio_read functions.
        uint8_t cur = gpio_read(STATUS_LED_PORT, STATUS_LED_PIN);
        if(cur == GPIO_PIN_HIGH){
            gpio_write(STATUS_LED_PORT, STATUS_LED_PIN, GPIO_PIN_LOW);
        } else {
            gpio_write(STATUS_LED_PORT, STATUS_LED_PIN, GPIO_PIN_HIGH);
        }
        last_toggle_time = system_ticks;
    }
}

void task_poll_button(void) {
    static uint8_t last_button_state = GPIO_PIN_HIGH;
    uint8_t current_button_state = gpio_read(USER_BUTTON_PORT, USER_BUTTON_PIN);

    if (current_button_state == GPIO_PIN_LOW && last_button_state == GPIO_PIN_HIGH) {
        // Button was just pressed (falling edge). For now, we do nothing.
        // In a real app, you might set a flag here.
    }
    last_button_state = current_button_state;
}

int main(void) {
    /* GPIO driver from 01-gpio-morse */
    // --- INITIALIZATION ---
    gpio_set_direction(STATUS_LED_PORT, STATUS_LED_PIN, GPIO_PIN_OUTPUT);
    gpio_set_direction(USER_BUTTON_PORT, USER_BUTTON_PIN, GPIO_PIN_INPUT);
    gpio_set_pullup(USER_BUTTON_PORT, USER_BUTTON_PIN, 1); // enable pull up
    
    system_tick_init();

    // --- SUPERLOOP ---
    while (1) {
        task_blink_status_led();
        task_poll_button();
    }
    return 0;
}

