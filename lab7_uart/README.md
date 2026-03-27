# Lab 7 – Debugging and User Interface
**Author:** Jabez Ian Macasero  
**Board:** ATmega328P  

---

## Lab Report Questions

### 1. Explain the purpose of the `UDRE0` flag. Why is it important to wait for it to be set before writing a new character to `UDR0`? What could happen if you didn't?

- The purpose of `UDRE0` flag in `UCSR0A` is it tells us whether the USART data register (UDR0) is empty and ready to accept a new byte. 
- We need to wait for this flag before writing because UDR0 can only hold one byte at a time. If we wrote a second byte before the first one finished shifiting out, the new byte overwrite the old one mid transimission.
- If I didn't, the result would be lost or corrupted characters in the serial output. 

### 2. What is the purpose of the `LDFLAGS` line added to the Makefile? Why is this necessary to make `printf` work correctly?

- The LDFLAGS line links in the full printf implementation. In other words, it's purpose is to define where external libraries are located or execute should be linkd, separate from compilation flags.

- Without these flags, our printf calls would either print nothing or print rubbish because the default stub does not actaully format anything

### 3. Our command interface is interrupt-driven. What is the main advantage of this approach compared to a polling approach where we would constantly check the `RXC0` flag in the main loop?

- The main advantage of the interrupt-driven approach is that we do not waste CPU cycles checking for incoming data.

- With polling, we would need to add a check for the RXC0 flag somewhere in our main loop, and if the loop is busy doing something else like reading the sensor or running the FSM, we might miss a byte entirely. This means our main loop stays clean and focused on the robot logic, and we never miss a command regardless of what the main loop is doing at that moment.