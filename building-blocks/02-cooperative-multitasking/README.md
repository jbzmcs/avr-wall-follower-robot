# 02 — Cooperative Multitasking on a System Tick

A 1 ms system tick drives non-blocking tasks in a superloop, replacing blocking delays.

- **System tick:** Timer1 in CTC mode, prescaler 64, `OCR1A = 249` → one compare
  interrupt per millisecond at 16 MHz. The ISR increments a `volatile` counter,
  `system_ticks`.
- **Tasks:** one blinks the status LED (PD5) every 500 ms; another polls a button (PD2)
  every pass. Neither ever blocks.

## Design notes

- **`volatile` is required.** `system_ticks` changes inside an ISR. Without `volatile`
  the compiler may cache it in a register, and the main loop would keep seeing a stale
  value.
- **`static` locals keep task state.** `last_toggle_time` lives inside the task function
  but survives between calls. As an ordinary local it would reset each call, and the LED
  would never time correctly.
- **Cooperative means every task must return fast.** There is no preemption: one slow
  task starves all the others. Tasks check the time and get out.

## Build

```sh
make && make flash
```
