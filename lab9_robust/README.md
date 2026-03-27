# Lab 9 – Failsafe and Optimization
**Author:** Jabez Ian Macasero  
**Board:** ATmega328P  

---

## Lab Report Questions

### 1. Why is it critical that the Watchdog Timer runs on its own independent oscillator, separate from the main system clock?

- If the WDT used the same clock as the CPU, then whatever bug or freeze that stops the CPU would also stop the watchdog timer. The whole point of the WDT is to detect when the main program has frozen and force a reset.

- Because it runs on its own internal 128kHz oscillator, even if the main 16MHz clock fails or the CPU locks up in an infinite loop, the WDT keeps counting independently and will still trigger the reset.

### 2. What is the difference between `eeprom_write_word` and `eeprom_update_word`? Why is the `update` version generally preferred for saving settings?

- eeprom_write_word always writes the value to EEPROM regardless of what is already stored there. eeprom_update_word reads the current value first and only writes if the new value is different.

- The update version is preferred because EEPROM has a limited number of write cycles (around 100,000). If we are saving the same value repeatedly, like saving the same target distance every time the user presses 'w', eeprom_write_word would wear out the EEPROM cells unnecessarily. eeprom_update_word skips the write if the value hasnt changed, extending the lifespan of the EEPROM.

### 3. Our robot wakes from sleep on a UART RX interrupt. What other interrupts are still active in `SLEEP_MODE_IDLE` that could also wake the CPU? (Hint: Think about the timers). Why is it important to choose a sleep mode that keeps necessary peripherals running?

- In SLEEP_MODE_IDLE the CPU halts but all the I/O clocks keep running. This means Timer0 (motor PWM), Timer1 (sensor TCNT1), Timer2 (servo PWM), and the USART are all still active. Any of their interrupts could wake the CPU, like a Timer0 overflow or a Timer2 compare match.

- It is important to choose this sleep mode specifically because our robot depends on those peripherals. If we used a deeper sleep mode like SLEEP_MODE_PWR_DOWN, the timers and USART would shut off completely. The UART RX interrupt would not work so we could never wake the robot with a serial command, and the timers would lose their configuration.