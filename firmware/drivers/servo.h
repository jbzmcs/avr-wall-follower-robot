// drivers/servo.h - Scan servo driver (Timer1 PWM on PB2 / Arduino D10)
#ifndef SERVO_H
#define SERVO_H

#include <stdint.h>

// Pulse widths in Timer1 counts (prescaler 64 -> 4us per count, 20ms frame).
#define SERVO_PULSE_RIGHT   150   // 0.6ms -> facing the right wall
#define SERVO_PULSE_FRONT   350   // 1.4ms -> facing forward
#define SERVO_PULSE_LEFT    625   // 2.5ms -> facing left (reference only)

void servo_init(void);
uint8_t servo_move(int16_t pulse);          // 1 if a move was started, 0 if already there
uint8_t servo_at_position(int16_t pulse);
void servo_sleep(void);                     // detach PWM so servo relaxes in idle
void servo_wake(void);

#endif
