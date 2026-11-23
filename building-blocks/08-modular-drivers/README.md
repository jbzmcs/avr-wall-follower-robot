# 08 — Modular Driver Architecture

Restructures the firmware into a `drivers/` folder (gpio, motor, sensor, uart) with
clean public APIs, leaving `main.c` for application logic only. This is the layout the
final firmware keeps.

## Design notes

- **Headers are the public interface.** They hold prototypes, shared types and the
  constants callers need. Implementations, static helpers and internal state stay in the
  `.c` file, so other modules can't depend on internals.
- **File-scope `static` for driver state.** `pulse_width` became `static` in `sensor.c`.
  It's still written by the ISR, but other files can only read it through
  `sensor_get_pulse_width()`, so nothing touches it at the wrong time.
- **Wildcard source lists.** `$(wildcard *.c) $(wildcard drivers/*.c)` in the Makefile
  picks up new drivers automatically. Adding `servo.c` later meant dropping one file in,
  with no build edits.

## Build

```sh
make && make flash
```
