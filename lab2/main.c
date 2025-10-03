
// Activity 2.1 Implementing a System Tick
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
