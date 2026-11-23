# 06 — Finite State Machine Wall Follower

The first stage where sensing and actuation work together. A three-state FSM
(`FORWARD`, `TURN_LEFT`, `TURN_RIGHT`) keeps the robot about 15 cm from a wall using
the ultrasonic sensor and the motor driver.

The sensor driver moves to a pin-change interrupt (PCINT0) on PB5 with Timer1
timestamps, and the motor driver adds Timer0 PWM speed control on the board's real pins.

## Design notes

- **Transition logic and action logic are separate.** One block decides the next state
  from the sensor reading; another decides what the motors do in each state. Each is
  easier to read and debug on its own.
- **Pivot turns vs. curve turns.** Pivoting (one wheel forward, one back) turns almost in
  place, which is good for tight corrections, but jerky and prone to oscillating with
  narrow thresholds. Curving (both wheels forward at different speeds) keeps the robot
  moving and follows more smoothly, at the cost of a wider turning radius.
- **The `_delay_ms(50)` sensing rate is blocking.** It could become a `millis()`
  timestamp check, the same pattern as stage 02.

## Build

```sh
make && make flash
```
