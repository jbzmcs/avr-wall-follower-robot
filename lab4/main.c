#define F_CPU 16000000UL
#include <util/delay.h>
#include "motor.h"
int main(void) {
    motor_init();
    while(1) {
        motor_forward(); _delay_ms(2000);
        motor_stop(); _delay_ms(1000);
        motor_backward(); _delay_ms(2000);
        motor_stop(); _delay_ms(1000);
        motor_turn_left(); _delay_ms(1000);
        motor_stop(); _delay_ms(1000);
        motor_turn_right(); _delay_ms(1000);
        motor_stop(); _delay_ms(1000);
    }
}