# Lab 2: Cooperative Multitasking

**Name:** Jabez Ian S. Macasero 
**Course:** COE185.1
**Date:** October 3, 2025

---

## Lab Report Questions

### Q1. What is the role of the `volatile` keyword, and what might happen if you removed it from the `system_ticks` declaration?

The `volatile` keyword tells the compiler that the variable can change unexpectedly, such as inside an interrupt service routine. Without it, the compiler may optimize by caching `system_ticks` in a register, causing the main loop to see old values. As a result, timing-based tasks like LED blinking (or button polling) would not behave correctly.

---

### Q2. Why is a `static` variable used for `last_toggle_time` inside `task_blink_status_led()`? What would happen if it were a regular local variable?

A `static` variable retains its value across function calls while remaining local in scope. This allows `last_toggle_time` to store the previous tick count and compare it against the current time. If it were a regular local variable, it would be reinitialized on every call, so the LED would never blink at the correct interval.

---

### Q3. This framework is "cooperative." What does that mean for how you must write your task functions? What is the main risk of this approach if one task takes too long to execute?

Cooperative multitasking means that each task must quickly finish its work and return control to the main loop so other tasks can run. Tasks should never block or use long delays. The main risk is that if one task takes too long, it will delay or starve the others, leading to missed events and reduced system responsiveness.

---

## Activities 2.1 & 2.2 Summary
**Activity 2.1:** Implemented a system tick using Timer1 in CTC mode with OCR1A = 249 for a 1 ms interrupt at 16 MHz with a prescaler of 64. A global `volatile` counter (`system_ticks`) was updated in the ISR. This provided the foundation for time-based scheduling of tasks.

**Activity 2.2:** Built two non-blocking tasks on top of the system tick. The first task blinked the status LED every 500 ms using `system_ticks` and a `static` variable to track elapsed time. The second task polled a user button by checking its state each loop iteration without delays. Both tasks ran in the main superloop, demonstrating cooperative multitasking.