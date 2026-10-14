# Terrarium Humidity PID

Discrete PID humidity control for a terrarium, using PWM-driven dual fans, a DHT11 sensor, and IRLZ44N MOSFETs on a [microcontroller: Arduino Uno / ESP32 / etc.].

![Demo](docs/images/demo.gif)

## Overview

This project keeps the relative humidity inside a terrarium at a desired setpoint. A DHT11 sensor measures the humidity, a discrete-time PID controller computes the control effort every sampling period, and the result is applied as a PWM duty cycle to two fans through logic-level MOSFETs. The fans exchange air with the surroundings to bring humidity toward the setpoint.

The goal of the project is to apply a discrete PID controller on a real, slow, noisy system and to document the practical issues that come with it, such as sensor resolution, sampling time selection, actuator saturation, and integral windup.

## System Description

The plant is the terrarium air volume, with relative humidity as the process variable. The sensor is a DHT11 (resolution of 1 %RH, update rate of about 1 Hz). The actuators are two fans driven by PWM through IRLZ44N MOSFETs, and the controller runs on [microcontroller] at a sampling time of [Ts] seconds. The humidity setpoint is [setpoint] %RH.

## Hardware

| Component | Qty | Notes |
|---|---|---|
| [Microcontroller] | 1 | |
| DHT11 humidity sensor | 1 | |
| IRLZ44N N-channel MOSFET | 2 | One per fan |
| [Fan type, voltage] | 2 | |
| [Power supply] | 1 | |
| Flyback diode, gate resistor, pull-down resistor | | [values] |

Wiring diagram: see `hardware/schematic.png`.

## Controller Design

The continuous PID law is

    u(t) = Kp*e(t) + Ki*integral(e(t) dt) + Kd*de(t)/dt

and it is discretized with [backward Euler / Tustin / forward Euler] at sampling time Ts. The resulting difference equation implemented in firmware is

    e[k]  = setpoint - humidity[k]
    I[k]  = I[k-1] + Ki*Ts*e[k]
    D[k]  = Kd*(e[k] - e[k-1])/Ts        (or derivative on measurement)
    u[k]  = Kp*e[k] + I[k] + D[k]

Implementation details:

- Output saturation: u[k] is limited to [0, 255] (or your PWM range).
- Anti-windup: [clamping / back-calculation] on the integral term.
- Derivative term: [derivative on measurement / low-pass filter, if used] to reduce noise and avoid derivative kick.
- Sampling time: Ts = [value] s, chosen because [reason, e.g. DHT11 update rate and the slow humidity dynamics].

## Tuning

Method: [Ziegler-Nichols / relay feedback / trial and error].

| Parameter | Value |
|---|---|
| Kp | [ ] |
| Ki | [ ] |
| Kd | [ ] |
| Ts | [ ] s |

## Results

Test conditions: [setpoint, initial humidity, ambient condition, duration].

| Metric | Value |
|---|---|
| Overshoot | [ ] % |
| Settling time | [ ] s |
| Steady-state error | [ ] %RH |

![Response plot](docs/images/response.png)

[One or two sentences interpreting the result, including any limitation such as DHT11 accuracy of about ±5 %RH.]

## Repository Structure

```
terrarium-humidity-pid/
├── README.md
├── LICENSE
├── firmware/
│   └── terrarium_pid.ino
├── hardware/
│   └── schematic.png
├── data/
│   └── test_log.csv
└── docs/
    └── images/
```

## Getting Started

1. Wire the circuit according to `hardware/schematic.png`.
2. Install the required libraries: [DHT sensor library, etc.].
3. Open `firmware/terrarium_pid.ino`, set the pin numbers and PID parameters at the top of the file.
4. Upload to the board and open the serial monitor at [baud rate] baud.

## Limitations and Future Work

- The DHT11 has coarse resolution and limited accuracy; a DHT22 or SHT3x would improve control quality.
- Fans can only reduce or exchange humidity, so there is no active humidification in the current design.
- Possible extensions: auto-tuning, a web-based dashboard, data logging.

## License

Released under the MIT License. See `LICENSE` for details.

## Author

Yefta Christian Nathaniel
GitHub: [@yeftchris](https://github.com/yeftchris)
Portfolio: [yeftchris.github.io](https://yeftchris.github.io)
