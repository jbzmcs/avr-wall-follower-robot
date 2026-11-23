# 10 — Integration & Tuning

All drivers integrated, with the robot tuned on real walls. A four-state FSM
(`IDLE`, `FORWARD`, `TURN_LEFT`, `TURN_RIGHT`) uses tiered fast/slow turn speeds, along
with the UART console, EEPROM target distance, watchdog and sleep mode.

## Lessons from getting here

- **The hardest step was the first full integration (stage 06).** Up to that point each
  piece worked in isolation. My LAFVIN board also differed from the reference design: a
  TB6612FNG with single direction pins instead of an L298N, and the ultrasonic echo on
  PB5 instead of PB0, which ruled out input capture. Debugging those mismatches while
  getting the FSM right took the most effort.
- **The modular architecture paid off here.** Adding the scan servo meant creating
  `servo.c`/`servo.h` and including the header; no other driver changed. When readings
  looked wrong, the bug was obviously in `sensor.c`, not somewhere in a large `main.c`.
- **Threshold steering hits a ceiling.** Going straight or turning at fixed speeds
  oscillates. The natural next step was a PD controller that steers in proportion to the
  distance error and its rate of change. That is exactly what the final
  [firmware](../../firmware) does.

## Build

```sh
make && make flash
```
