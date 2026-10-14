## How the Controller Works

### From continuous PID to discrete PID

The textbook PID law is written in continuous time:

    u(t) = Kp*e(t) + Ki*∫e(t)dt + Kd*de(t)/dt

A microcontroller does not watch the humidity continuously. It takes one measurement every Ts = 1 second and computes one output per measurement. Discretizing the law means replacing the two calculus operations with their sampled equivalents: the integral becomes a running sum of error multiplied by Ts, and the derivative becomes the difference between two consecutive errors divided by Ts.

### Symbols

| Symbol | Meaning | Value in this project |
|---|---|---|
| SP | Humidity setpoint | 70 %RH |
| RH[k] | Measured humidity at sample k | from DHT11 |
| e[k] | Error at sample k | SP - RH[k] |
| Ts | Sampling time | 1 s |
| Kp, Ki, Kd | Proportional, integral, derivative gains | 8.0, 0.08, 0.0 |
| I[k] | Accumulated (integrated) error | clamped to ±100 |
| u[k] | Controller output | PWM units |
| B | PWM balance point | 80 |

### Step 1: Error

    e[k] = SP - RH[k]

The error tells the controller how far the humidity is from the target and in which direction. A positive error means the air is too dry (RH below 70 %), a negative error means it is too humid.

### Step 2: Proportional term

    P[k] = Kp * e[k]

The proportional term reacts to the present error. With Kp = 8, each 1 %RH of error adds or removes 8 PWM units. It responds immediately but cannot, on its own, remove a small steady error, because a tiny error produces a tiny correction.

### Step 3: Integral term

    I[k] = I[k-1] + e[k]*Ts
    I[k] is limited to the range -100 to +100
    Integral contribution = Ki * I[k]

The integral term remembers the past. Every second it adds the current error multiplied by Ts, so a small error that persists keeps accumulating until the output is strong enough to remove it. This is what drives the steady-state error toward zero. The limit of ±100 is the anti-windup: it stops the sum from growing without bound while the fans are saturated. Because Ki = 0.08, the integral can contribute at most 0.08 * 100 = 8 PWM units.

### Step 4: Derivative term

    D[k] = (e[k] - e[k-1]) / Ts
    Derivative contribution = Kd * D[k]

The derivative term looks at how fast the error is changing and acts against fast changes. In this project Kd = 0, so this term contributes nothing and the controller behaves as a PI controller. It is kept in the code so it can be enabled for experiments. Note that the DHT11 has only 1 %RH resolution, so its readings change in steps, which makes the derivative noisy and is one reason it is left off.

### Step 5: Controller output

    u[k] = Kp*e[k] + Ki*I[k] + Kd*D[k]

The three contributions are added into one number u[k]. It is signed: positive when the terrarium is too dry, negative when it is too humid.

### Step 6: Balancing the two fans

Instead of mapping u[k] to a single actuator, the output is split between two fans around the balance point B = 80:

    PWM_intake[k]  = B + u[k]
    PWM_exhaust[k] = B - u[k]

When u = 0, both fans run at the same duty cycle and the airflow is balanced. When u is positive, the intake fan speeds up and the exhaust fan slows down by the same amount. When u is negative, the opposite happens. The balance point keeps both fans running at moderate speed so the controller can move in either direction without ever fully stopping the airflow.

### Step 7: Saturation

    PWM = constrain(PWM, 0, 255)

An 8-bit PWM output only accepts values from 0 to 255, so each fan command is clamped to that range. With B = 80, the exhaust fan reaches 0 when u ≥ 80 and the intake fan reaches 255 when u ≥ 175. Since Kp = 8, the exhaust fan is already fully off at an error of roughly 10 %RH.

### Worked example

Assume the controller has just started (I = 0) and the sensor reads RH = 62 %.

    e = 70 - 62 = 8
    P = 8 * 8 = 64
    I = 0 + 8*1 = 8        ->  Ki*I = 0.08 * 8 = 0.64
    D = 0                  (Kd = 0)
    u = 64 + 0.64 + 0 = 64.64

    PWM_intake  = 80 + 64.64 = 144.64  ->  144
    PWM_exhaust = 80 - 64.64 = 15.36   ->  15

The air is too dry, so the intake fan runs fast (144) while the exhaust fan almost stops (15). If instead the sensor reads RH = 75 % on the first sample, then e = -5, u = -40.4, and the commands become intake = 39 and exhaust = 120: the controller now pushes air out faster than it brings air in. The firmware converts the result to an integer, so the decimal part is dropped.

On each following second the integral keeps accumulating the error, and the humidity gradually approaches 70 % while the two PWM values converge back toward the balance point.
