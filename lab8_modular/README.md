# Lab 8 – Modular Firmware Design
**Author:** Jabez Ian Macasero  
**Board:** ATmega328P  

---

## Lab Report Questions

### 1. What is the primary purpose of a header (`.h`) file in a modular C project? What kind of information should go in it, and what should not?

- The purpose of a header file is to declare the public interface of a module so other files know what functions and types are available without seeing the actual implementation.

- A header file should contain function prototypes, type definitions like enums and structs, and any constants that other modules need to use.

- What should not go in a header file is the actual function implementations, static helper functions, or internal variables that only the module itself uses. This keeps the codebase organized and prevents other files from depending on internal details.

### 2. In `sensor.c`, the `pulse_width` variable was changed from a global variable to a `static` global variable. What does the `static` keyword do in this context, and why is this good practice for a driver module?

- The static keyword on a global variable limits its visibility to the file it is declared in. Without static, pulse_width would be a regular global that any other file could access directly using an extern declaration.

- By making it static, only sensor.c can see and modify pulse_width. Any other file that needs the value has to go through the public API function sensor_get_pulse_width().

- This is good practice because it prevents other parts of the code from accidentally reading or writing to the variable at the wrong time, especially since it gets modified inside an ISR.

### 3. Explain what the `$(wildcard *.c)` function does in the Makefile. Why is this a more scalable approach than manually listing every source file?

- The `$(wildcard *.c)` function tells Make to automatically find all files ending in .c in the specified directory. In our Makefile we use `$(wildcard *.c)` `$(wildcard drivers/*.c)` to pick up main.c from the root and all driver source files from the drivers folder.

- This is more scalable because when we add a new driver like servo.c we just drop it in the drivers folder and the Makefile picks it up automatically. With a manual list we would have to remember to update the SRC line every time we add or rename a file, which is easy to forget and causes linker errors.