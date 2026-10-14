# Terrarium Humidity PID

Discrete-time PI(D) humidity controller for a terrarium. An Arduino reads relative humidity from a DHT11 sensor and drives an intake fan and an exhaust fan in opposite directions around a balance point, using PWM.

![Demo](docs/images/demo.gif)

## Overview

The controller holds the terrarium humidity at a setpoint of 70 %RH. Every second, the Arduino measures humidity, computes a discrete PID output, and shifts PWM between two fans: when humidity is below the setpoint, the intake fan speeds up and the exhaust fan slows down, and the opposite happens when humidity is above the setpoint. Because both fans move around a common balance duty cycle, the airflow is regulated without ever switching the fans fully off.

The project documents the practical side of discrete control on a slow, noisy system: sampling time, sensor resolution, actuator saturation, and integral windup.

## Hardware

| Component | Qty | Notes |
|---|---|---|
| Arduino [board type] | 1 | PWM on pins 9 and 10 |
| DHT11 humidity sensor | 1 | Data pin on D2 |
| IRLZ44N N-channel MOSFET | 2 | One per fan, gate driven by PWM |
| Fan [voltage / size] | 2 | Intake (D9) and exhaust (D10) |
| [Power supply] | 1 | |

Wiring diagram: see `hardware/schematic.png`.

## Control Algorithm

The error is defined as e[k] = SP - RH[k], so a positive error means the terrarium is too dry.

Discrete terms, with sampling time Ts = 1 s:

    I[k] = I[k-1] + e[k] * Ts          (rectangular integration)
    D[k] = (e[k] - e[k-1]) / Ts        (backward difference)
    u[k] = Kp*e[k] + Ki*I[k] + Kd*D[k]

Balancing output to the two fans:

    PWM_intake[k]  = PWM_BALANCE + u[k]
    PWM_exhaust[k] = PWM_BALANCE - u[k]

Both are clamped to the range 0 to 255.

Implementation notes:

- Sampling is done non-blocking with `millis()`, so the loop does not use `delay()`.
- Integral anti-windup: the integral state is clamped to ±100.
- If the DHT11 returns NaN, that sample is skipped and the previous output is kept.
- Kd is set to 0, so in the current configuration the controller behaves as a PI controller. The derivative term is implemented and can be enabled for experiments.

## Parameters

| Parameter | Value |
|---|---|
| Setpoint | 70 %RH |
| Kp | 8.0 |
| Ki | 0.08 |
| Kd | 0.0 |
| Ts | 1 s |
| PWM balance | 80 |
| Integral limit | ±100 |

Tuning method: [Ziegler-Nichols / relay feedback / trial and error].

## Results

Test conditions: [initial humidity, ambient condition, duration].

| Metric | Value |
|---|---|
| Overshoot | [ ] % |
| Settling time | [ ] s |
| Steady-state error | [ ] %RH |

![Response plot](docs/images/response.png)

## Serial Output

The firmware prints CSV at 9600 baud, which can be logged and plotted directly:

    RH,SP,Error,PID,IntakePWM,ExhaustPWM

## Repository Structure

```
terrarium-humidity-pid/
├── README.md
├── LICENSE
├── firmware/
│   └── PID_Diskrit_Terrarium.ino
├── hardware/
│   └── schema-hard.pdf
├── data/
│   └── test_log.xlsx
└── docs/
    └── images/
```

## Getting Started

1. Wire the circuit according to `hardware/schematic.png`.
2. Install the DHT sensor library by Adafruit (and its Adafruit Unified Sensor dependency).
3. Open `firmware/PID_Diskrit_Terrarium.ino` and adjust the setpoint and PID parameters at the top of the file.
4. Upload to the Arduino and open the serial monitor at 9600 baud.

## Limitations and Future Work

- The DHT11 has 1 %RH resolution and roughly ±5 %RH accuracy; a DHT22 or SHT3x would improve control quality.
- The anti-windup only clamps the integral state; conditional integration or back-calculation would be more robust.
- Derivative on error can cause derivative kick if Kd is increased; derivative on measurement with a low-pass filter is a possible improvement.
- Possible extensions: auto-tuning, a web-based dashboard, data logging.

## License

Released under the MIT License. See `LICENSE` for details.

## Author

Yefta Christian Nathaniel
GitHub: [@yeftchris](https://github.com/yeftchris)
Portfolio: [yeftchris.github.io](https://yeftchris.github.io)
