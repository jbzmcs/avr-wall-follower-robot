# COE185 - Embedded Systems Labs - Jabez Ian S. Macasero

This repository contains my lab activities for **COE185 - Embedded Systems**.
Each lab is organized into its own folder (`lab0`, `lab1`, …, `lab10`) and includes source code, a Makefile, and documentation.

---

## Repository Structure

* **lab0/** – Blink LED on PB5 (intro to AVR programming)
* **lab1/** – The Bare-Metal Foundation
* **lab2/** – Cooperative Multitasking
* **lab3/** – Reactivity with Interrupts
* **lab4/** – Motor Control and PWM
* **lab5/** – Sensing with Advanced Timers
* **lab6/** – Logic and Control — FSM
* **lab7/** – Debugging and User Interface
* **lab8/** – Modular Firmware Design
* **lab9/** – Failsafe and Optimization
* **lab10/** – Final Project Integration & Tuning

---

## How to Use

1. Navigate to the desired lab folder, e.g.:

   ```bash
   cd lab0
   ```
2. Compile the source code with the provided `Makefile`:

   ```bash
   make
   ```
3. Upload the program to the microcontroller:

   ```bash
   make flash
   ```

---

## Notes

* Target device: **ATmega328p**
* Toolchain: **avr-gcc**, **avrdude**
* IDE/Editor: **VS Code**
* Each lab folder has its own `README.md` with detailed documentation.

---

## References

* [Course Lab Instructions](http://stephenhaim.com/classes/coe185/)
* Mazidi et al., *AVR Microcontroller and Embedded Systems*
