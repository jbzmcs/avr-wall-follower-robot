# Lab 3 – Reactivity with Interrupts  
**Author:** Jabez Ian Macasero  
**Board:** ATmega328P  

---

## Lab Report Questions

### Q1. Explain the complete sequence of events, from the physical button press to the LED changing state, mentioning the roles of the hardware (INT0), the ISR, the volatile flag, and the main loop tasks.
When the button connected to **PD2 (INT0)** is pressed, the hardware detects a **falling edge** and automatically triggers the **INT0 interrupt**. The **ISR(INT0_vect)** runs immediately and sets a global **volatile flag (`g_button_pressed_flag`)**, signaling that an event occurred.  
Back in the main loop, the **`task_process_button_press()`** function periodically checks this flag. If the flag is set and the debounce time has passed, it toggles the **`g_led_is_blinking`** state, clears the flag, and records the timestamp. The **`task_blink_led()`** function then blinks or turns off the LED based on this state.  

In summary:
- **Hardware (INT0):** detects button press via falling edge  
- **ISR:** sets volatile flag quickly  
- **Volatile flag:** ensures shared variable updates are visible to main loop  
- **Main loop tasks:** handle debounce and LED state changes  

---

### Q2. Why is the debounce logic handled in the main loop task (`task_process_button_press`) instead of directly inside the ISR with a `_delay_ms()`? Refer to the "Golden Rule of ISRs."
The debounce logic is handled in the **main loop** instead of inside the ISR because of the **Golden Rule of ISRs:**  
> *"Keep ISRs short, simple, and fast."*  
Using `_delay_ms()` inside the ISR would block other interrupts (like the timer ISR that updates `system_ticks`) and make the system unresponsive.  
By keeping the ISR minimal and using time-based debounce in the main loop, the program stays **non-blocking** and responsive to all events.

---

### Q3. The `EICRA` register can be configured to trigger on "any logical change." What is a potential disadvantage of this mode for a simple push-button compared to "falling edge"?
If INT0 is configured to trigger on **any logical change**, the interrupt would fire on both the **press** and **release** of the button, as well as on electrical **bounces** that occur during each transition.  
For a mechanical push-button, this results in **multiple unintended triggers**, making debounce more difficult. Using the **falling-edge** mode ensures the ISR runs only once per press, providing a cleaner and more stable response.

---

## Activities 3.1 & 3.2 Summary
Activity **3.1** introduced configuring an **external interrupt (INT0)** to detect button presses on PD2.  
Activity **3.2** integrated this interrupt-driven input into the **multitasking framework** from Lab 2. The ISR sets a flag when the button is pressed, while main loop tasks handle **debouncing**, **state toggling**, and **LED blinking** without blocking the system timer.

---

## Code Behavior
- **INT0** triggers on a falling edge when the button is pressed.  
- The ISR sets `g_button_pressed_flag` (no delays or logic).  
- The main loop checks the flag and toggles `g_led_is_blinking` after a 50 ms debounce period.  
- When `g_led_is_blinking` is active, the LED on **PD5** blinks every 500 ms; otherwise, it stays off.  
- The system remains fully non-blocking and responsive due to time-based task scheduling.

---
