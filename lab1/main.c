#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

#define DOT_LENGTH_MS 250

void dot() {
    PORTD |= (1 << PORTD2);     // LED ON
    _delay_ms(DOT_LENGTH_MS);
    PORTD &= ~(1 << PORTD2);    // LED OFF
    _delay_ms(DOT_LENGTH_MS);   // gap
}

void dash() {
    PORTD |= (1 << PORTD2);
    _delay_ms(DOT_LENGTH_MS * 3);
    PORTD &= ~(1 << PORTD2);
    _delay_ms(DOT_LENGTH_MS);
}

int main(void) {
    // LED as output
    DDRD |= (1 << DDD2);

    // Buttons as inputs with pull-ups
    DDRB &= ~((1 << DDB3) | (1 << DDB4) | (1 << DDB5));
    PORTB |= (1 << PORTB3) | (1 << PORTB4) | (1 << PORTB5);

    while (1) {
        // Button M (--)
        if (!(PINB & (1 << PINB3))) {
            dash(); dash();
            _delay_ms(DOT_LENGTH_MS * 2);
        }

        // Button S (...)
        if (!(PINB & (1 << PINB4))) {
            dot(); dot(); dot();
            _delay_ms(DOT_LENGTH_MS * 2);
        }

        // Button U (..-)
        if (!(PINB & (1 << PINB5))) {
            dot(); dot(); dash();
            _delay_ms(DOT_LENGTH_MS * 2);
        }
    }
    return 0;
}
