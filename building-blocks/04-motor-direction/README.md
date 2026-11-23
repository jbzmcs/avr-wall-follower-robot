# 04 — Motor Direction Control

A first motor driver with an API that the later stages build on:
`motor_forward()`, `motor_backward()`, `motor_turn_left()`, `motor_turn_right()` and
`motor_stop()`. The demo cycles through each motion with pauses in between.

This stage uses the L298N-style four-pin direction scheme (IN1–IN4 on PD3, PD4, PD7,
PB0). My chassis turned out to use a TB6612FNG with one direction pin per motor
(PD2, PD4) and PWM on PD5/PD6, so from stage 06 onward the driver keeps the same API
but adds Timer0 PWM speed control and the board's actual pins.

## Build

```sh
make && make flash
```
