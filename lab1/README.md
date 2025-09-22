# Lab 1 – The Bare-Metal Foundation (Morse Code with GPIO Driver)

## Lab Report Questions

### 1. Explain the purpose of the bitwise OR (`|=`) and bitwise AND with inversion (`&= ~`) for setting and clearing bits in a register. Why is this safer than writing a full value (e.g., `DDRD = 0b01000000;`)?

* `|=` (bitwise OR assignment) is used to **set** a bit to `1` without changing the other bits in the register.
* `&= ~` (bitwise AND with inverted mask) is used to **clear** a bit to `0` while leaving other bits unchanged.
* This is safer than writing an entire value (like `DDRD = 0b01000000;`) because direct assignment overwrites all bits in the register, potentially changing unrelated pins. Using bit masking only modifies the intended bit while preserving the state of all other bits.

---

### 2. Why is the button's logic "active low" (i.e., we check if the pin is `LOW` to detect a press)? What is the role of the internal pull-up resistor in this configuration?

* The button is active low because one side of the button is connected to ground. When pressed, it pulls the input pin to `0` (logic LOW).
* The internal pull-up resistor ensures the input pin is at a defined `HIGH` state (`1`) when the button is **not pressed**. Without the pull-up, the pin would "float," leading to unpredictable values. Thus, the pull-up provides a stable default state, and pressing the button overrides it to LOW.

---

### 3. What is the major limitation of using `_delay_ms()` for timing in a more complex application where the robot needs to perform multiple tasks at once?

* `_delay_ms()` is a **blocking delay**: while waiting, the CPU cannot perform any other work.
* In a simple blinking LED demo, this is fine, but in a more complex robot, blocking delays prevent multitasking (e.g., reading sensors, updating motors, handling communication).
* The limitation is scalability: once the system grows, blocking delays make the program unresponsive and inefficient. Timers and interrupts are needed for concurrent task handling.

---

### Activity 1.1 – Functional Logic

* Implemented Morse code blinker using direct register manipulation.
* Used buttons on PB3, PB4, PB5 to send Morse for `M` (`--`), `S` (`...`), and `U` (`..-`).
* LED output on PD2.
* Verified functionality against rubric using blocking delays and bitwise register operations.

### Activity 1.2 – Modular Design

* Created a GPIO driver (`gpio.c` and `gpio.h`) with the following functions:

  * `gpio_set_direction()` – configure pin as input or output
  * `gpio_write()` – set pin HIGH or LOW
  * `gpio_set_pullup()` – enable/disable internal pull-up resistor
  * `gpio_read()` – return pin state as HIGH or LOW
* Refactored `main.c` to only use driver functions, eliminating direct register manipulation.
* Updated `Makefile` to include `gpio.c` in compilation and linking.
* Verified program correctness: pressing each button lights the LED with the correct Morse sequence.

---

## How to Build and Flash

```bash
make clean
make
make flash
```

* `make` builds the ELF and HEX files.
* `make flash` uploads the program to the ATmega328P using `avrdude`.
* Ensure the correct COM port is set in the Makefile (`-P COM3` by default).
