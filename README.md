# AVR Wall-Follower Robot

An autonomous robot that follows a wall at a set distance, steering with a PD
controller and avoiding obstacles with a servo-mounted ultrasonic sensor. Written in
bare-metal C for the ATmega328P: direct register access, hand-written drivers, and
no Arduino libraries.

<!-- Demo: add a photo or GIF of the robot here, e.g. ![Robot demo](docs/demo.gif) -->

## Features

- **PD wall following.** Steering is proportional to the distance error and damped by
  its rate of change, so the robot holds its distance smoothly instead of zig-zagging
  between thresholds.
- **Scan-servo sensing.** The ultrasonic sensor stays aimed at the wall and swings
  forward every 150 ms to check for obstacles.
- **Recovery behaviors.** It pivots away from obstacles until the path is clear, curves
  back to find a lost wall, and backs off if it gets too close.
- **Serial console.** Start, stop and adjust the target distance live over UART at
  9600 baud, with `printf` telemetry.
- **Persistent settings.** The target distance is saved in EEPROM.
- **Failsafes.** A hardware watchdog resets the MCU on a hang, and idle sleep mode wakes
  on a serial command.

## Hardware

| Component | Part |
|---|---|
| MCU | ATmega328P @ 16 MHz |
| Chassis | LAFVIN smart-car kit with a TB6612FNG motor driver |
| Distance sensor | HC-SR04 ultrasonic |
| Scan servo | SG90-class micro servo (signal on D10) |

All three hardware timers are in use: Timer0 for motor PWM, Timer1 for the servo, and
Timer2 for the 1 ms system tick. The full pin map and timer table are in the
[firmware docs](firmware/README.md).

## Repository layout

```
├── firmware/            The complete robot: state machine, PD controller, drivers
└── building-blocks/     The subsystems built up one at a time on the way there
```

The robot was built incrementally. Each stage in `building-blocks/` is a standalone,
buildable program that adds one capability:

| Stage | Adds |
|---|---|
| [01-gpio-morse](building-blocks/01-gpio-morse) | GPIO driver, bitwise register access |
| [02-cooperative-multitasking](building-blocks/02-cooperative-multitasking) | 1 ms timer tick, non-blocking tasks |
| [03-external-interrupts](building-blocks/03-external-interrupts) | INT0 button input, time-based debouncing |
| [04-motor-direction](building-blocks/04-motor-direction) | Motor driver API |
| [05-ultrasonic-sensing](building-blocks/05-ultrasonic-sensing) | HC-SR04 timing with input capture |
| [06-fsm-wall-follower](building-blocks/06-fsm-wall-follower) | First wall follower: FSM, PWM speed, pin-change echo timing |
| [07-uart-console](building-blocks/07-uart-console) | Serial console, `printf` debugging |
| [08-modular-drivers](building-blocks/08-modular-drivers) | `drivers/` architecture |
| [09-watchdog-eeprom-sleep](building-blocks/09-watchdog-eeprom-sleep) | Watchdog, EEPROM settings, sleep mode |
| [10-integration-tuning](building-blocks/10-integration-tuning) | Full integration and tuning |

## Build & flash

Requires `avr-gcc`, `avr-libc` and `avrdude`.

```sh
cd firmware
make          # build out/main.hex
make flash    # upload over the Arduino bootloader
```

The programmer port is set in each `Makefile` (`-P COMx`); change it to match your
machine.

## Background

This started as coursework for an embedded systems course and grew into a complete
robot. The final control design is adapted from a friend's working wall-follower build;
I ported it into this driver architecture and added the console, EEPROM settings,
watchdog and sleep mode. See [firmware/README.md](firmware/README.md#9-design-provenance).

**Reference:** Mazidi et al., *The AVR Microcontroller and Embedded Systems*.

## License

[MIT](LICENSE)
