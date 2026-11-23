# 09 — Watchdog, EEPROM Settings & Sleep Mode

Robustness features carried into the final firmware:

- **Watchdog:** a 1 s watchdog resets the MCU if the main loop ever hangs.
- **EEPROM:** the target wall distance persists across power cycles. `+` and `-` adjust
  it over UART, and `w` saves it.
- **Sleep:** when paused, the robot enters `SLEEP_MODE_IDLE` and wakes on a UART command.

## Design notes

- **The watchdog has its own oscillator.** It runs from an independent 128 kHz clock, so
  even if the CPU locks up or the main clock fails, it keeps counting and still resets
  the chip.
- **`eeprom_update_word`, not `eeprom_write_word`.** Update reads first and writes only if
  the value changed. EEPROM cells survive about 100,000 writes, so skipping redundant
  saves extends their life.
- **Choosing the sleep depth.** Idle mode halts the CPU but keeps the I/O clocks running,
  so timers and the USART stay configured, and a UART byte can wake the robot. Any
  still-enabled timer interrupt can wake it too. Power-down would stop the USART, and a
  serial command could no longer wake it.

## Build

```sh
make && make flash
```
