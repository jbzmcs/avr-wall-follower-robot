# Lab 6 – Logic and Control - The Finite State Machine
**Author:** Jabez Ian Macasero  
**Board:** ATmega328P  

---

## Lab Report Questions


### 1. Why is separating the FSM's "state transition logic" from its "state action logic" a good software design practice?

- Separating the transition logic from the action logic keeps the code organized and easier to debug. The transition block only decides what state to move to based on the sensor input, and the action block only cares about what the motors should do in each state.

### 2. Our FSM uses "pivot turns" (one wheel forward, one backward). What is another type of turn the robot could make, and what would the motor actions be for it? What are the advantages or disadvantages of that type of turn?
- Another type of turn is a curve turn where one wheel runs at full speed and the other runs at a reduced speed instead of reversing. 
- Pivot turns can rotate almost in place which is better for tight corrections, but they are jerky and can cause the robot to oscillate back and forth if the thresholds are too narrow.
- The advantage is that the robot keeps moving forward while turning, so it covers ground and follows the wall more smoothly. The disadvantage is that the turning radius is much wider than a pivot turn, so the robot cannot correct as quickly if it gets very close to the wall. 


### 3. The `_delay_ms(50)` in the main loop is a simple way to control the sensing rate, but it is a blocking call. How could you implement this delay in a non-blocking way using the system tick from Lab 2?
- Instead of calling _delay_ms(50) which freezes the entire CPU for 50 milliseconds, we could use a millis() function from the system tick timer set up in Lab 2.
We would save a timestamp at the start and then on each loop iteration check if enough time has passed. Something like storing last_trigger_time = millis() when we trigger the sensor, and then only triggering again when millis() - last_trigger_time >= 50. This way the CPU is free to do other work during those 50 milliseconds instead of sitting in an empty loop doing nothing. It is the same non-blocking pattern we learned in Lab 2 with the LED blink.