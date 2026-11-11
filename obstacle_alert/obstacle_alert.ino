/*
 * Obstacle-alert glasses - POC firmware (ESP32, Arduino core 3.x)
 *
 * Mirrors the group's alert flowchart:
 *   Ultrasonic sensor -> ESP32 -> process sensor data -> object in range?
 *   Battery           -> ESP32 -> battery low?
 *   Either one        -> create alert -> send alert -> buzzer + vibration motor
 *                                                    (+ headphones if connected)
 *
 * Wiring (adjust pins below to match your board):
 *   HC-SR04  TRIG -> GPIO 5
 *   HC-SR04  ECHO -> GPIO 18  (through a voltage divider if the sensor is 5V, see notes)
 *   Vibration motor -> GPIO 25 -> resistor -> NPN transistor base (motor on collector,
 *                      flyback diode across the motor). Never drive the motor straight
 *                      from a GPIO pin.
 *   Active buzzer   -> GPIO 26
 *   Battery sense   -> GPIO 34 from a 2:1 divider (2 x 100k) on the battery +
 */

// ---------------------------- Pins ----------------------------
constexpr uint8_t TRIG_PIN   = 5;
constexpr uint8_t ECHO_PIN   = 18;
constexpr uint8_t MOTOR_PIN  = 25;
constexpr uint8_t BUZZER_PIN = 26;
constexpr uint8_t BATT_PIN   = 34;   // ADC1, input-only pin

// ---------------------------- Tuning ----------------------------
constexpr float    ALERT_RANGE_CM      = 150.0;  // start alerting inside this distance
constexpr float    NEAR_CM             = 30.0;   // at or inside this = strongest/fastest alert
constexpr uint8_t  CONFIRM_READINGS    = 2;      // consecutive in-range readings required (filters noise)
constexpr uint32_t SENSOR_INTERVAL_MS  = 60;     // time between distance readings
constexpr uint32_t ECHO_TIMEOUT_US     = 25000;  // ~4m max wait for an echo

constexpr uint32_t PULSE_PERIOD_FAR_MS  = 600;   // slow pulses when object is far
constexpr uint32_t PULSE_PERIOD_NEAR_MS = 80;    // rapid pulses when object is near
constexpr uint32_t PULSE_ON_MS          = 40;    // how long each pulse lasts
constexpr uint8_t  DUTY_FAR             = 120;   // vibration strength (0-255) when far
constexpr uint8_t  DUTY_NEAR            = 255;   // vibration strength when near

constexpr uint32_t MOTOR_PWM_FREQ = 1000;
constexpr uint8_t  MOTOR_PWM_BITS = 8;

// Battery monitoring: leave false until the voltage divider is actually wired,
// otherwise the floating pin will read garbage and trigger false low-battery alerts.
constexpr bool     ENABLE_BATTERY_MONITOR = false;
constexpr float    DIVIDER_RATIO          = 2.0;   // 2 x 100k divider
constexpr int      LOW_BATT_MV            = 3500;  // LiPo "getting low" - tune this
constexpr int      BATT_HYSTERESIS_MV     = 100;
constexpr uint32_t BATTERY_INTERVAL_MS    = 5000;
constexpr uint32_t LOW_BATT_REPEAT_MS     = 30000;

constexpr bool DEBUG_SERIAL = true;  // prints readings to the Serial Monitor for testing

// ---------------------------- Alert model ----------------------------
enum AlertType { ALERT_NONE, ALERT_OBSTACLE, ALERT_LOW_BATTERY };

struct Alert {
  AlertType type;
  float distanceCm;  // only meaningful for ALERT_OBSTACLE
};

// ---------------------------- State ----------------------------
float    lastDistanceCm   = -1;
uint8_t  inRangeCount     = 0;
bool     objectInRange    = false;

int      batteryMv        = 0;
bool     batteryLow       = false;

uint32_t lastSensorMs     = 0;
uint32_t lastBatteryMs    = 0;
uint32_t lastLowBattMs    = 0;
uint32_t pulseStartMs     = 0;

// ---------------------------- Setup ----------------------------
void setup() {
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  // Arduino-ESP32 core 3.x PWM API (see notes for 2.x)
  ledcAttach(MOTOR_PIN, MOTOR_PWM_FREQ, MOTOR_PWM_BITS);
  ledcWrite(MOTOR_PIN, 0);

  Serial.println("Obstacle alert system ready");
}

// ---------------------------- Main loop ----------------------------
void loop() {
  uint32_t now = millis();

  // Battery branch of the flowchart: measure voltage -> is battery low?
  if (ENABLE_BATTERY_MONITOR && now - lastBatteryMs >= BATTERY_INTERVAL_MS) {
    lastBatteryMs = now;
    checkBattery();
  }

  // Sensor branch: continuous stream of data -> process -> object in range?
  if (now - lastSensorMs >= SENSOR_INTERVAL_MS) {
    lastSensorMs = now;
    processSensorData();
  }

  Alert alert = createAlert();
  sendAlert(alert);
}

// ---------------------------- Sensor ----------------------------
// Returns distance in cm, or -1 if no echo came back (nothing in range).
float readDistanceCm() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  unsigned long duration = pulseIn(ECHO_PIN, HIGH, ECHO_TIMEOUT_US);
  if (duration == 0) return -1;

  // Speed of sound ~0.0343 cm/us; divide by 2 for the round trip
  return duration * 0.0343f / 2.0f;
}

// "Process sensor data" + "Is object in range?" boxes
void processSensorData() {
  float cm = readDistanceCm();
  lastDistanceCm = cm;

  bool hit = (cm > 0 && cm <= ALERT_RANGE_CM);
  if (hit) {
    if (inRangeCount < 255) inRangeCount++;
  } else {
    inRangeCount = 0;
  }
  objectInRange = (inRangeCount >= CONFIRM_READINGS);

  if (DEBUG_SERIAL) {
    Serial.printf("dist=%.1f cm  inRange=%d  batt=%d mV  low=%d\n",
                  cm, objectInRange, batteryMv, batteryLow);
  }
}

// ---------------------------- Battery ----------------------------
// "Measure voltage to determine battery life" + "Is battery low?" boxes
void checkBattery() {
  batteryMv = (int)(analogReadMilliVolts(BATT_PIN) * DIVIDER_RATIO);

  // Hysteresis so the flag doesn't flicker around the threshold
  if (!batteryLow && batteryMv < LOW_BATT_MV) {
    batteryLow = true;
  } else if (batteryLow && batteryMv > LOW_BATT_MV + BATT_HYSTERESIS_MV) {
    batteryLow = false;
  }
}

// ---------------------------- Alerts ----------------------------
// "Create alert" box. Obstacles take priority over low-battery warnings.
Alert createAlert() {
  Alert a = { ALERT_NONE, -1 };

  if (objectInRange) {
    a.type = ALERT_OBSTACLE;
    a.distanceCm = lastDistanceCm;
  } else if (batteryLow && millis() - lastLowBattMs >= LOW_BATT_REPEAT_MS) {
    a.type = ALERT_LOW_BATTERY;
    lastLowBattMs = millis();
  }
  return a;
}

// "Send alert" box -> buzzer + vibration motor, then headphones if connected
void sendAlert(const Alert &a) {
  switch (a.type) {
    case ALERT_NONE:
      setOutputs(false, 0);
      break;
    case ALERT_OBSTACLE:
      driveObstacleAlert(a.distanceCm);
      break;
    case ALERT_LOW_BATTERY:
      lowBatteryPattern();
      break;
  }

  // "Are headphones connected?" branch
  if (a.type != ALERT_NONE && headphonesConnected()) {
    sendToHeadphones(a);
  }
}

// Closer object = faster pulses and stronger vibration.
void driveObstacleAlert(float cm) {
  float c = constrain(cm, NEAR_CM, ALERT_RANGE_CM);
  uint32_t period = map((long)c, (long)NEAR_CM, (long)ALERT_RANGE_CM,
                        PULSE_PERIOD_NEAR_MS, PULSE_PERIOD_FAR_MS);
  uint8_t duty = map((long)c, (long)NEAR_CM, (long)ALERT_RANGE_CM,
                     DUTY_NEAR, DUTY_FAR);

  uint32_t now = millis();
  if (now - pulseStartMs >= period) pulseStartMs = now;

  bool on = (now - pulseStartMs) < PULSE_ON_MS;
  setOutputs(on, duty);
}

// Two short beeps. Blocking, but it only runs once every 30s and never while
// an obstacle alert is active, so it's fine for the POC.
void lowBatteryPattern() {
  for (int i = 0; i < 2; i++) {
    setOutputs(true, 200);
    delay(150);
    setOutputs(false, 0);
    delay(150);
  }
}

void setOutputs(bool on, uint8_t duty) {
  ledcWrite(MOTOR_PIN, on ? duty : 0);
  digitalWrite(BUZZER_PIN, on ? HIGH : LOW);
}

// ---------------------------- Headphones (placeholder) ----------------------------
// Not implemented for the POC. Later this could check a Bluetooth audio connection
// (or a 3.5mm jack-detect pin) and send panned/pitched tones for direction + distance.
bool headphonesConnected() {
  return false;
}

void sendToHeadphones(const Alert &a) {
  // TODO: audio output for bone-conduction / open-ear headphones
}
