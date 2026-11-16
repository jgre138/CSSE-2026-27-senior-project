// TEST: Buzzer - turns on for 1 second, off for 1 second
// Wiring: Buzzer + -> GPIO 26, Buzzer - -> GND (active buzzer)

constexpr uint8_t BUZZER_PIN = 14; // This pin is at GIOP14 or the 15th pin on the ESP32 DevKitC board. You can change it to any other GPIO pin that is available for your specific board.

void setup() {
  Serial.begin(115200);
  pinMode(BUZZER_PIN, OUTPUT);
}

void loop() {
  Serial.println("Buzzer ON");
  digitalWrite(BUZZER_PIN, HIGH);
  delay(1000);

  Serial.println("Buzzer OFF");
  digitalWrite(BUZZER_PIN, LOW);
  delay(1000);
}
