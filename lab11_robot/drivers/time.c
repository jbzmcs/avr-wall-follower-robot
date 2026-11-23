// time.c - Timer2 CTC 1ms tick providing millis()/micros()
#include <avr/io.h>
#include <avr/interrupt.h>
#include "time.h"

// Timer2 in CTC mode, prescaler 64, OCR2A = 249:
// 16MHz / 64 = 250kHz -> 250 ticks = 1ms per compare match.
static volatile uint32_t g_ms_count = 0;

ISR(TIMER2_COMPA_vect) {
    g_ms_count++;
}

void time_init(void) {
    TCCR2A = (1 << WGM21);              // CTC mode, TOP = OCR2A
    TCCR2B = (1 << CS22);               // prescaler 64
    OCR2A  = 249;                       // 1ms period
    TIMSK2 |= (1 << OCIE2A);            // enable compare interrupt
}

void time_pause(void) {
    TIMSK2 &= ~(1 << OCIE2A);
}

void time_resume(void) {
    TIMSK2 |= (1 << OCIE2A);
}

uint32_t millis(void) {
    uint32_t t;
    uint8_t sreg = SREG;   // read the 32-bit counter atomically

    cli();
    t = g_ms_count;
    SREG = sreg;

    return t;
}

uint32_t micros(void) {
    uint32_t m;
    uint8_t t;
    uint8_t sreg = SREG;   // snapshot counter + timer atomically

    cli();
    m = g_ms_count;
    t = TCNT2;
    SREG = sreg;

    // Each Timer2 tick is 4us at prescaler 64 / 16MHz.
    return (m * 1000UL) + ((uint32_t)t * 4UL);
}
