### COE185-M89 
# Lab 0 - Blink the LED on Port B, bit 5

**Author: Jabez Ian S. Macasero**

---

## Lab Overview

This lab introduces AVR programming by toggling an LED connected to **Port B, bit 5 (PB5)**. The task demonstrates the use of data direction registers, port manipulation, and software delays.

---

## Objectives

* Familiarize with AVR I/O registers (`DDRB`, `PORTB`).
* Implement LED blinking using delay functions.
* Compile and upload code using `avr-gcc` and `avrdude`.

---

## Implementation

* Configured PB5 as output using `DDRB`.
* Turned LED on/off by writing to `PORTB`.
* Used `_delay_ms(500)` to create a half-second blink interval.
* Automated build and flashing with Makefile.

---

## Results

The LED connected to PB5 blinks with a 1-second cycle (500 ms on, 500 ms off), verifying correct register configuration and program execution.

---

## Conclusion

Lab 0 successfully introduced register-level AVR programming. I gained hands-on practice with GPIO control, delay usage, and build automation with `avr-gcc` and `avrdude`.
