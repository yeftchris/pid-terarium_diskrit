#include <DHT.h>

// ==================================================
// PIN
// ==================================================
#define DHT_PIN 2
#define DHT_TYPE DHT11

#define INTAKE_PIN 9
#define EXHAUST_PIN 10

// ==================================================
// SETPOINT
// ==================================================
const float SETPOINT = 70.0;

// ==================================================
// PWM
// ==================================================
const int PWM_BALANCE = 80;
const int PWM_MIN = 0;
const int PWM_MAX = 255;

// ==================================================
// PI DISKRIT
// ==================================================
float Kp = 8.0;
float Ki = 0.08;
float Kd = 0.0;

// Sampling Time Diskrit
const float Ts = 1.0;  // detik

float integral = 0;
float prevError = 0;

const float INTEGRAL_LIMIT = 100;

// ==================================================
// TIMING
// ==================================================
unsigned long lastTime = 0;
const unsigned long SAMPLE_TIME = 1000;  // 1 detik

// ==================================================
DHT dht(DHT_PIN, DHT_TYPE);

// ==================================================
void setup() {
  Serial.begin(9600);

  pinMode(INTAKE_PIN, OUTPUT);
  pinMode(EXHAUST_PIN, OUTPUT);

  dht.begin();

  analogWrite(INTAKE_PIN, PWM_BALANCE);
  analogWrite(EXHAUST_PIN, PWM_BALANCE);

  Serial.println("RH,SP,Error,PID,IntakePWM,ExhaustPWM");
}

// ==================================================
void loop() {
  unsigned long now = millis();

  if (now - lastTime >= SAMPLE_TIME) {
    lastTime = now;

    // =============================================
    // BACA SENSOR
    // =============================================
    float rh = dht.readHumidity();

    if (isnan(rh))
      return;

    // =============================================
    // HITUNG ERROR
    // =============================================
    float error = SETPOINT - rh;

    // =============================================
    // INTEGRAL DISKRIT
    // I(k)=I(k-1)+e(k)Ts
    // =============================================
    integral += error * Ts;

    integral = constrain(
      integral,
      -INTEGRAL_LIMIT,
      INTEGRAL_LIMIT);

    // =============================================
    // DERIVATIVE DISKRIT
    // =============================================
    float derivative =
      (error - prevError) / Ts;

    // =============================================
    // OUTPUT PID
    // =============================================
    float pidOutput =
      (Kp * error) + (Ki * integral) + (Kd * derivative);

    // =============================================
    // BALANCING CONTROL
    // =============================================
    int intakePWM =
      PWM_BALANCE + pidOutput;

    int exhaustPWM =
      PWM_BALANCE - pidOutput;

    intakePWM = constrain(
      intakePWM,
      PWM_MIN,
      PWM_MAX);

    exhaustPWM = constrain(
      exhaustPWM,
      PWM_MIN,
      PWM_MAX);

    // =============================================
    // OUTPUT KE AKTUATOR
    // =============================================
    analogWrite(INTAKE_PIN, intakePWM);
    analogWrite(EXHAUST_PIN, exhaustPWM);

    // =============================================
    // SERIAL MONITOR
    // =============================================
    Serial.print(rh);
    Serial.print(",");

    Serial.print(SETPOINT);
    Serial.print(",");

    Serial.print(error);
    Serial.print(",");

    Serial.print(pidOutput);
    Serial.print(",");

    Serial.print(intakePWM);
    Serial.print(",");

    Serial.println(exhaustPWM);

    // =============================================
    // UPDATE ERROR
    // =============================================
    prevError = error;
  }
}
