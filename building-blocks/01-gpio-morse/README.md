# 01 — GPIO Driver & Morse Code Blinker

Bare-metal GPIO on the ATmega328P. Three buttons send Morse code letters on an LED:
PB3 → `M` (`--`), PB4 → `S` (`...`), PB5 → `U` (`..-`), with the LED on PD2.

Started with direct register access, then refactored behind a small GPIO driver
that every later stage reuses:

| Function | Purpose |
|---|---|
| `gpio_set_direction()` | Configure a pin as input or output |
| `gpio_write()` | Drive a pin HIGH or LOW |
| `gpio_set_pullup()` | Enable/disable the internal pull-up |
| `gpio_read()` | Read a pin as HIGH or LOW |

## Design notes

- **Read-modify-write, not assignment.** `|=` sets a bit and `&= ~` clears one
  while leaving every other bit alone. Writing a whole value (`DDRD = 0b01000000;`)
  silently reconfigures unrelated pins.
- **Active-low buttons with pull-ups.** Each button shorts its pin to ground. The
  internal pull-up holds the pin at a defined HIGH when released, so it never floats,
  and a press reads as LOW.
- **Blocking delays don't scale.** `_delay_ms()` keeps the timing simple here, but the
  CPU can do nothing else while it waits. The next stage replaces it with a timer.

## Build

```sh
make && make flash
```
