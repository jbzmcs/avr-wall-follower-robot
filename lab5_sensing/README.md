## Lab 5 Report Questions

### 1. Why is using the hardware Input Capture Unit (ICU) more accurate for measuring pulse width than a software polling approach?

- With a polling approach, the CPU is stuck in a loop checking if the pin changed, which means by the time it actually detects the edge, a few instruction cycles have already passed. That delay is inconsistent because other things like interrupts can push it even further. The measured time is always a little late and never the same amount of late.
- The ICU fixes this by doing the timestamp in hardware. The moment the edge happens on ICP1, the hardware immediately copies `TCNT1` into `ICR1` — no CPU involvement, no delay. It captures the exact timer value at the exact moment of the transition.
- At prescaler 8 and 16 MHz, one tick is 0.5 µs. The ICU is accurate to that resolution. Software polling can't get close to that because you're always at least a few instructions behind.

### 2. In `TIMER1_CAPT_vect`, why is it necessary to toggle the `ICES1` bit? What would happen if you didn't?

- The ICU can only watch for one edge at a time — either rising or falling, set by the `ICES1` bit in `TCCR1B`. To measure pulse width you need two timestamps: one when the echo goes HIGH and one when it goes LOW. So you have to switch edge detection mode inside the ISR after each capture.
- On the first trigger (rising edge), you save `ICR1` as `start_time` and toggle `ICES1` to now watch for the falling edge. On the second trigger, you compute `pulse_width = ICR1 - start_time`.
- If you forgot to toggle it, the ISR would keep firing on the same edge every time. You'd end up comparing the start of one measurement to the start of the next one instead of measuring the actual echo pulse. The distance calculation would be completely wrong.

### 3. How could you rewrite the main loop to be non-blocking instead of using `_delay_ms(60)`?

- Instead of blocking for 60ms, I would record the system tick right after calling `sensor_trigger()` into something like `last_trigger_ms = get_tick()`. Then in the main loop, instead of `_delay_ms(60)`, I'd check `if ((get_tick() - last_trigger_ms) >= 60)` before triggering the next measurement.
- The loop would run freely the whole time. Between sensor triggers, other tasks like updating motor speeds or checking UART commands could run without waiting on the delay. This is the same cooperative multitasking pattern from Lab 2 — use the system tick as a non-blocking timer instead of halting execution.