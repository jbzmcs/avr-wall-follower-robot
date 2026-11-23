# 03 — External Interrupts with Debouncing

Adds an interrupt-driven button to the multitasking framework. A press on PD2 (INT0)
toggles the status LED (PD5) between blinking every 500 ms and off.

## How it works

1. **Hardware:** INT0 is configured for a falling edge, so a press fires the interrupt.
2. **ISR:** `ISR(INT0_vect)` only sets the `volatile` flag `g_button_pressed_flag`.
3. **Main loop:** `task_process_button_press()` sees the flag, waits out a 50 ms debounce
   window, toggles `g_led_is_blinking`, and clears the flag.
4. `task_blink_led()` blinks or turns off the LED based on that state.

## Design notes

- **Keep ISRs short.** Debouncing with `_delay_ms()` inside the ISR would block the
  system-tick interrupt and freeze timing for everything else. The ISR records the event,
  and the time-based debounce happens in a task.
- **Falling edge, not "any change".** Triggering on any logical change fires on press,
  release, and every contact bounce, which makes debouncing much harder. Falling edge
  fires once per press.

## Build

```sh
make && make flash
```
