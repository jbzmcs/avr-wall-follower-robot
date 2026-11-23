# 05 — Ultrasonic Distance Sensing (Input Capture)

An HC-SR04 driver that times the echo pulse with Timer1's hardware Input Capture Unit
on ICP1 (PB0). The distance is computed from the pulse width, and a debug LED on PD5
blinks when something is closer than 20 cm.

## Design notes

- **Hardware capture beats polling.** Polling detects the edge a few instructions late,
  and the delay varies whenever other interrupts run. The ICU copies `TCNT1` into `ICR1`
  at the exact moment of the edge, with no CPU involvement. At prescaler 8 that is
  0.5 µs resolution.
- **Toggle `ICES1` between edges.** The ICU watches one edge at a time. The ISR records
  the rising edge, switches `ICES1` to falling, and then computes
  `pulse_width = ICR1 - start_time`. Without the toggle it would measure from one pulse's
  start to the next one's.
- **Next step: drop the blocking delay.** The `_delay_ms(60)` between triggers could be
  replaced by checking `millis() - last_trigger_ms >= 60` so other tasks run in between.

> **Hardware note:** on my LAFVIN board the echo pin is wired to PB5, not PB0, and ICP1
> can't be remapped. Later stages time the echo with a pin-change interrupt (PCINT0)
> and timer timestamps instead.

## Build

```sh
make && make flash
```
