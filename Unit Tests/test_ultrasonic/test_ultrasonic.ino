// TEST: Ultrasonic sensor (HC-SR04) - prints what it reads
// Wiring: VCC -> 5V, GND -> GND, TRIG -> GPIO 5,
//         ECHO -> GPIO 18 through a voltage divider (1k from ECHO to the pin, 2k from the pin to GND)

constexpr uint8_t TRIG_PIN = 26;
constexpr uint8_t ECHO_PIN = 27;

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  Serial.println("Connected:, will run shortly.....");
  delay(1000);
}

void loop() {
  // Send a short pulse
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Measure how long the echo takes to come back (0 = no echo)
  unsigned long duration = pulseIn(ECHO_PIN, HIGH, 25000);

  if (duration == 0) {
    Serial.println("No echo");
  } else {
    Serial.printf("Distance: %.1f cm\n", duration * 0.0343 / 2);
  }
  delay(200);
}
