# Firmware — Continuous Wall-Following Robot

An autonomous wall-following robot on the ATmega328P (LAFVIN chassis). The robot
follows a wall on its **right side** at a configurable target distance using a
servo-mounted ultrasonic sensor and a **PD steering controller**, while
periodically scanning forward for obstacles. Includes a UART command console,
EEPROM-persisted settings, a hardware watchdog, and an idle sleep mode.

---

## 1. Hardware

| Component | Part | Connection |
|---|---|---|
| MCU | ATmega328P @ 16 MHz | — |
| Motor driver | TB6612FNG (on-board) | see pin map |
| Distance sensor | HC-SR04 ultrasonic | Trig PB4, Echo PB5 |
| Scan servo | SG90-class hobby servo | Signal on **PB2 / Arduino D10** |
| Console | USB-UART | 9600 baud, 8N1 |

### Pin map

| Signal | AVR pin | Arduino pin | Peripheral |
|---|---|---|---|
| Left motor direction (DIRA) | PD2 | D2 | GPIO |
| Left motor PWM (PWMA) | PD5 | D5 | Timer0 OC0B |
| Right motor direction (DIRB) | PD4 | D4 | GPIO |
| Right motor PWM (PWMB) | PD6 | D6 | Timer0 OC0A |
| Servo signal | **PB2** | **D10** | Timer1 OC1B |
| Ultrasonic Trig | PB4 | D12 | GPIO |
| Ultrasonic Echo | PB5 | D13 | PCINT5 (pin-change interrupt) |
| UART TX/RX | PD1/PD0 | D1/D0 | USART0 |

> **Wiring note:** earlier revisions of this project drove the servo from
> PB3 / D11. The final revision moved it to **PB2 / D10**. Verify the signal wire before
> flashing.

### Timer allocation

Every timer on the ATmega328P is spoken for; nothing may be reused without
breaking a subsystem:

| Timer | Mode | Owner | Purpose |
|---|---|---|---|
| Timer0 (8-bit) | Fast PWM, /64 | `drivers/motor.c` | Motor speed on OC0A/OC0B |
| Timer1 (16-bit) | Fast PWM mode 15, /64, TOP = OCR1A = 4999 | `drivers/servo.c` | Exact 50 Hz servo frame, 4 µs pulse resolution on OC1B |
| Timer2 (8-bit) | CTC, /64, OCR2A = 249 | `drivers/time.c` | 1 ms system tick → `millis()` / `micros()` |

The 16-bit Timer1 is deliberately assigned to the servo: it yields ~475
discrete positions across the sweep versus ~30 on an 8-bit timer. The
ultrasonic echo is timed with `micros()` (4 µs resolution ≈ 0.7 mm), which is
far below sensor noise, so it does not need a hardware capture timer.

---

## 2. Project layout

```
firmware/
├── main.c              Control logic: FSM, PD controller, filtering, UART console
├── Makefile            Build/flash (avr-gcc, avrdude on COM6)
├── out/                Build artifacts (main.hex is the flash image)
└── drivers/
    ├── gpio.c/.h       Port-abstracted pin control
    ├── motor.c/.h      TB6612FNG: direction + Timer0 PWM speed
    ├── servo.c/.h      Timer1 PWM servo with position tracking and sleep
    ├── sensor.c/.h     HC-SR04 read via PCINT + micros() timestamps
    ├── time.c/.h       Timer2 1 ms tick, millis()/micros(), pause/resume
    └── uart.c/.h       9600-baud USART, printf stream, RX interrupt enabled
```

Layering rule: `main.c` never touches registers for sensing/actuation — it
calls driver APIs. Drivers own their timer and pins exclusively.

---

## 3. Control design

### 3.1 Units

All control math runs in **inches** (floating point). The user-facing target
is stored and adjusted in **centimeters** (integer, EEPROM-persisted) and
converted once whenever it changes:

```
g_target_in = g_target_distance_cm / 2.54        // default 23 cm ≈ 9.06 in
```

### 3.2 Sensing schedule

The servo *lives* pointed at the right wall. Each `RUNNING` iteration takes a
**two-sample averaged** side reading (10 ms apart, invalid samples dropped).
Every **150 ms** (`FRONT_CHECK_INTERVAL_MS`, timed with `millis()`), the robot:

1. slows to `MIN_SPEED` (so it is not driving fast while blind),
2. swings the servo forward, settles 100 ms, takes one quick reading,
3. swings back to the wall and resumes.

This wall-priority schedule delivers several side samples per second, which is
what makes tight continuous steering possible.

### 3.3 Filtering

Side readings pass through a 3-tap moving average (`apply_filter`):

- valid readings (1.5–80 in) enter a circular history; the mean of valid
  entries is returned;
- an invalid reading (0.0) does **not** pollute the history — the last average
  is returned instead (or the target distance if the history is empty);
- 4 consecutive invalid *raw* readings (`bad_reading_count >
  BAD_READS_FOR_LOST`) mean the wall is genuinely gone → `STATE_FIND_WALL`.

### 3.4 PD steering controller

Executed on every `RUNNING` iteration with the filtered wall distance:

```
error      = right_distance − g_target_in          // + = too far, − = too close
derivative = error − previous_error
steering   = KP·error + KD·derivative              // clamped to ±MAX_STEERING

left  = base_speed + steering                      // + steering veers RIGHT (toward wall)
right = base_speed − steering                      // − steering veers LEFT  (away)
```

Both wheel speeds are clamped to `[MIN_SPEED, MAX_SPEED]`. The proportional
term sizes the correction to the error; the derivative term opposes the rate
of change, damping the oscillation a P-only controller would exhibit.

Base speed adapts to alignment:

| Condition | Base speed |
|---|---|
| \|error\| < 2.5 in (well aligned) | `MAX_SPEED` = 0.32 |
| \|error\| ≤ 3.0 in | `CRUISE_SPEED` = 0.25 |
| \|error\| > 3.0 in (way off) | `MIN_SPEED` = 0.15 |

Speeds are fractions of full power; `main.c` converts them to Timer0 PWM
counts with a ceiling of `MOTOR_PWM_TOP` = 200 (of 255).

---

## 4. State machine

```
                 's' cmd                        valid 1st reading
   ┌──────┐  ──────────────►  ┌─────────┐  ─────────────────────►  ┌─────────┐
   │ IDLE │                   │ STARTUP │                          │ RUNNING │◄─┐
   └──────┘  ◄──────────────  └─────────┘                          └─┬─┬─┬───┘  │
      ▲          'p' cmd                                             │ │ │      │
      │  (from any state via UART)             front < 14 in ────────┘ │ │      │
      │                                                                │ │      │
      │        ┌────────────────┐   front clear                        │ │      │
      └─       │ AVOID_OBSTACLE │ ─────────────────────────────────────┼─┼──────┤
               └────────────────┘   (re-pivots until clear)            │ │      │
                                               wall > 18 in ───────────┘ │      │
               ┌───────────┐        wall < 18 in                         │      │
               │ FIND_WALL │ ────────────────────────────────────────────┼──────┤
               └───────────┘   (keeps curving right until found)         │      │
                                               wall < 2 in ──────────────┘      │
               ┌────────────────┐   after back-up + pivot                       │
               │ EMERGENCY_STOP │ ──────────────────────────────────────────────┘
               └────────────────┘
```

| State | Behavior | Exit condition |
|---|---|---|
| `IDLE` | Motors off, servo parked forward then detached, MCU in `SLEEP_MODE_IDLE`. Watchdog and ms-tick paused. | UART `'s'` → `STARTUP` |
| `STARTUP` | Servo to wall, waits for one valid filtered reading before any motion. | Valid reading → `RUNNING` |
| `RUNNING` | Continuous PD wall-following + periodic front scan (§3). | Obstacle / wall-lost / danger transitions (see diagram) |
| `AVOID_OBSTACLE` | Stop → pivot left in-place 200 ms → stop → re-check front. **Repeats** until front ≥ 14 in, then creeps forward 150 ms and resumes. | Front clear → `RUNNING` |
| `FIND_WALL` | Curves right (`left 0.25 / right 0.15`) in 200 ms steps, taking a fresh averaged reading between steps. | Reading < 18 in → `RUNNING` |
| `EMERGENCY_STOP` | Stop → reverse 200 ms → pivot left 150 ms → re-measure → resume. Recovers *distance*, not just heading. | Always → `RUNNING` |

Transitions are printed to UART as they happen: `FSM: RUNNING -> FIND_WALL`.

### Key thresholds (inches)

| Constant | Value | Meaning |
|---|---|---|
| `OBSTACLE_TH_IN` | 14.0 | Front reading below this = obstacle, avoid |
| `WALL_LOST_TH_IN` | 18.0 | Side reading above this = wall lost, seek |
| `DISTANCE_TOLERANCE` | 2.5 | "Well aligned" band → allows `MAX_SPEED` |
| `EMERGENCY_STOP_DIST` | 2.0 | Side reading below this = back off now |
| `BAD_READS_FOR_LOST` | 3 | Consecutive invalid reads before seeking |

---

## 5. Robustness features

### UART console (9600 baud)

| Key | Action |
|---|---|
| `s` | Start: wake servo, reset filter/controller, enter `STARTUP` |
| `p` | Pause: stop motors, enter `IDLE` (sleep) |
| `+` / `-` | Raise / lower target distance by 1 cm (live, takes effect immediately) |
| `w` | Persist current target to EEPROM |

Commands are handled in the `USART_RX_vect` ISR, so `p` works instantly even
mid-maneuver.

### EEPROM settings

The target distance lives at a fixed EEMEM address. On boot, `settings_load()`
reads it; a blank cell (0xFFFF) is initialized to the 23 cm default.
`eeprom_update_word` is used throughout so unchanged saves cost no write cycles.

### Watchdog

A 2-second watchdog (`WDTO_2S`) is armed after init. `wdt_reset()` is called
once per main-loop iteration **and** inside every long blocking maneuver
(avoidance pivots, emergency recovery, front-scan block). A hang anywhere
resets the MCU into the safe boot state (motors off, `IDLE`). The watchdog is
disabled during intentional sleep and re-armed on wake.

### Sleep mode

In `IDLE` the robot: stops motors → parks the servo forward → detaches the
servo PWM pin (`servo_sleep()`, so the servo draws no hold current and stops
twitching) → pauses the Timer2 tick (`time_pause()`, so the 1 ms interrupt
cannot wake the core) → enters `SLEEP_MODE_IDLE`. The only wake source left is
UART RX. On wake everything is restored in reverse order.

---

## 6. Driver reference

### motor.h
```c
void motor_init(void);                      // Timer0 fast PWM /64, pins out, stopped
void motor_forward(void);                   // DIRA=1, DIRB=0
void motor_backward(void);
void motor_turn_left(void);                 // left wheel back, right wheel fwd (pivot)
void motor_turn_right(void);
void motor_set_speeds(uint8_t l, uint8_t r);// raw PWM counts (OCR0B / OCR0A)
void motor_stop(void);
```
Direction and speed are orthogonal: direction functions never reset speed.

### servo.h
```c
#define SERVO_PULSE_RIGHT 150   // 0.6 ms  → facing right wall
#define SERVO_PULSE_FRONT 350   // 1.4 ms  → facing forward
#define SERVO_PULSE_LEFT  625   // 2.5 ms  → facing left (unused)

void    servo_init(void);               // Timer1 mode 15, 50 Hz, boots facing front
uint8_t servo_move(int16_t pulse);      // returns 0 if already there (skips settle)
uint8_t servo_at_position(int16_t pulse);
void    servo_sleep(void);              // detach OC1B, pin low
void    servo_wake(void);
```
Positions are raw pulse counts (4 µs each), not degrees — the three constants
are calibrated to this servo horn's mounting.

### sensor.h
```c
void   sensor_init(void);           // PCINT5 on echo, trig low
double sensor_read_inches(void);    // blocking, ≤15 ms; 0.0 = timeout/out-of-range
```
One call does everything: 10 µs trigger, echo edges timestamped with
`micros()` in the `PCINT0` ISR, 15 ms timeout, validity window 1.5–80 in.

### time.h
```c
void     time_init(void);      // Timer2 CTC 1 ms tick
uint32_t millis(void);
uint32_t micros(void);         // 4 µs resolution
void     time_pause(void);     // stop the tick interrupt (used around sleep)
void     time_resume(void);
```
`millis()`/`micros()` reads are interrupt-safe (SREG save + `cli`).

### uart.h / gpio.h
`uart_init()` + `uart_get_stream()` binds `printf` to the serial port
(`\n` → `\r\n` translation included); RX interrupt is enabled by init.
`gpio_*` provides port-abstracted pin direction/read/write used by all drivers.

---

## 7. Building and flashing

Requirements: `avr-gcc`, `avr-libc`, `avrdude`. Programmer configured in the
Makefile as Arduino bootloader on **COM6** @ 115200.

```sh
make            # build → out/main.hex
make flash      # build + upload via avrdude
make clean      # remove out/
```

Current footprint: **11.8 KB flash (36 %), 1.2 KB RAM (58 %)**. The RAM figure
includes the float-capable printf (`-lprintf_flt`) needed for `%.2f` debug
output.

### First-run checklist

1. Servo signal wire on **D10** (not D11 — see §1).
2. Open a serial terminal at **9600 baud**.
3. Expect the boot banner, EEPROM target report, and `Send 's' to start.`
4. Send `s` — servo swings right, `Startup R: x.xx in` appears, robot starts
   once the reading is valid.
5. Watch one front-check cycle (`F: x.xx in` every ~150 ms of run time).

---

## 8. Tuning guide

All knobs are `#define`s at the top of `main.c`. The shipped values are the
endurance-validated baseline — change one thing at a time and keep notes.

| Symptom | Knob | Direction |
|---|---|---|
| Weaves / oscillates along a straight wall | `KD` up (more damping) or `KP` down | small steps: KP ±0.005, KD ±0.005 |
| Sluggish return to target after a disturbance | `KP` up | — |
| Corrections saturate (always full tilt) | `MAX_STEERING` down | — |
| Clips inside corners | `OBSTACLE_TH_IN` up (turn earlier) | — |
| Turns away from walls it should follow | `WALL_LOST_TH_IN` up | — |
| Too slow overall | `CRUISE_SPEED` / `MAX_SPEED` up | re-verify cornering after |
| Overshoots outside corners while seeking | `FIND_WALL` step: lower left speed or shorter `_delay_ms(200)` step | — |

---

## 9. Design provenance

The control constants, FSM structure, and sensing schedule are adapted from a
friend's working wall-follower build, which passed a long endurance run
(~99 % success). I ported that design into this project's layered driver
architecture and added the UART console, EEPROM settings, watchdog, and sleep
mode. If experimentation degrades behavior, the values documented in §3–§4 are
the known-good configuration to restore.
