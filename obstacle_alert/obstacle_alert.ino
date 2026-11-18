// Obstacle alert - ESP32
// Reads the ultrasonic sensor. If something is within range, the buzzer and
// vibration motor pulse. The closer the object, the faster the pulses.
//
// Wiring (same as the unit tests):
//   HC-SR04: VCC -> 5V, GND -> GND, TRIG -> GPIO 26,
//            ECHO -> GPIO 27 through a voltage divider (1k from ECHO to the pin, 2k from the pin to GND)
//   Vibration motor: GPIO 17 -> ~1k resistor -> transistor base (motor on the collector, flyback diode)
//   Buzzer (active): GPIO 23 -> buzzer +, buzzer - -> GND
//   We have a passive buzzer, which is SUPER quiet, so we will need to use a different buzzer or get an active buzzer to get a louder sound?

constexpr uint8_t TRIG_PIN = 26;
constexpr uint8_t ECHO_PIN = 27;
constexpr uint8_t MOTOR_PIN  = 17;
constexpr uint8_t BUZZER_PIN = 23;

constexpr float ALERT_RANGE_CM = 150;  // start alerting inside this distance
constexpr float NEAR_CM        = 30;   // at or inside this distance = fastest pulses
constexpr int   SLOW_PERIOD_MS = 600;  // time between pulses when the object is far
constexpr int   FAST_PERIOD_MS = 80;   // time between pulses when the object is near
constexpr int   PULSE_ON_MS    = 40;   // how long each pulse lasts

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(MOTOR_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
}

// Returns distance in cm, or -1 if no echo came back
float readDistanceCm() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  unsigned long duration = pulseIn(ECHO_PIN, HIGH, 25000);
  if (duration == 0) return -1;
  return duration * 0.0343 / 2;
}

// Turns the buzzer and vibration motor on or off together
void setAlert(bool on) {
  digitalWrite(MOTOR_PIN, on);
  digitalWrite(BUZZER_PIN, on);
}

void loop() {
  float cm = readDistanceCm();
  Serial.println(cm);

  if (cm > 0 && cm <= ALERT_RANGE_CM) {
    // Object in range: closer = shorter wait between pulses
    float c = constrain(cm, NEAR_CM, ALERT_RANGE_CM);
    int period = map((long)c, (long)NEAR_CM, (long)ALERT_RANGE_CM, FAST_PERIOD_MS, SLOW_PERIOD_MS);

    setAlert(true);
    delay(PULSE_ON_MS);
    setAlert(false);
    delay(period - PULSE_ON_MS);
  } else {
    // Nothing in range
    setAlert(false);
    delay(60);
  }
}
