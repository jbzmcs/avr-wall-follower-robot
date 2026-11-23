# 07 — UART Console & printf Debugging

Adds a 9600-baud serial console. `printf` is routed to the UART for live state and
distance logging, and an interrupt-driven receiver accepts `s` to start and `p` to
pause the robot.

## Design notes

- **Wait for `UDRE0`.** `UDR0` holds one byte. Writing a new byte before the last one
  has shifted out corrupts or drops characters, so the transmitter waits for the
  data-register-empty flag.
- **printf needs the full library.** The Makefile's `LDFLAGS` links avr-libc's complete
  `vfprintf`. The default stub doesn't format anything, so output would be empty or
  garbage.
- **Interrupt-driven receive.** With `ISR(USART_RX_vect)`, no command is missed while the
  main loop is busy with sensing or the FSM. Polling `RXC0` wastes cycles and can drop
  bytes.

## Build

```sh
make && make flash
```
