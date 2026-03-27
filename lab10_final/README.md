# Lab 10 – Final Project Integration & Demonstration
**Author:** Jabez Ian Macasero  
**Board:** ATmega328P  

---

## Final Reflection Questions

### 1. Looking back at the entire project, what was the most challenging lab and why?

- The most challenging lab for me was Lab 6 because it was the first time everything had to work together. Before that each lab was isolated, I was just testing motors or just testing the sensor. But in Lab 6 the FSM had to read the sensor and control the motors at the same time, and any small mistake in one driver would break the whole system.

- On top of that, my LAFVIN expansion board has different pin mappings from the lab manual which assumes an L298N H-bridge. I had to figure out that my board uses single DIO direction pins (PD2, PD4) instead of IN1/IN2/IN3/IN4 pairs, and that the ultrasonic sensor echo is on PB5 instead of PB0 so I could not use Timer1 Input Capture. Debugging those hardware mismatches while also trying to get the FSM logic right made Lab 6 the hardest one.

### 2. Explain why the modular, driver-based architecture from Lab 8 was critical for making the final system integration in this lab manageable.

- Without the modular architecture, main.c would have been hundreds of lines of mixed motor control, sensor timing, UART logic, servo code, and FSM logic all tangled together. If something broke it would have been nearly impossible to figure out which part caused the problem.

- With the driver-based structure, each piece of hardware has its own file with a clean public API. When I needed to add the servo for the supplemental instruction, I just created servo.c and servo.h, dropped them in the drivers folder, and included the header in main.c. I did not have to touch motor.c or sensor.c at all. The same thing applied when debugging, if the sensor readings were wrong I knew to look in sensor.c, not dig through a giant main.c file.

### 3. If you had one more week, what new feature or improvement would you add to your robot, and which software modules would you need to modify to implement it?

- I would add a PD (proportional-derivative) controller for the wall-following steering instead of the simple threshold-based approach. Right now the robot either goes straight, turns left, or turns right at fixed speeds. A PD controller would calculate a smooth steering correction based on how far off the target distance we are and how fast the error is changing, which would make the robot follow the wall much more smoothly without the oscillation I saw during testing.

- I would only need to modify main.c to replace the threshold checks and fixed motor speeds in the FSM with the PD calculation. The motor driver, sensor driver, and servo driver would all stay exactly the same since the PD controller just outputs different speed values to motor_set_speeds().