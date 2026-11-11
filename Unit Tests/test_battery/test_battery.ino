// TEST: Battery level - prints the battery voltage and a rough percent
// Wiring: Battery + -> 100k resistor -> GPIO 34 -> 100k resistor -> GND
//         Battery - -> ESP32 GND (grounds must be shared)
// No battery yet? Connect the board's 5V pin to the top of the divider to test the setup (expect ~5V).
// Note: the percent only makes sense for a single LiPo cell (3.3V-4.2V).

constexpr uint8_t BATT_PIN = 34;
constexpr float DIVIDER_RATIO = 2.0;  // 2 x 100k resistors

void setup() {
  Serial.begin(115200);
}

void loop() {
  uint32_t pinMv = analogReadMilliVolts(BATT_PIN);
  float battMv = pinMv * DIVIDER_RATIO;
  int percent = constrain(map((long)battMv, 3300, 4200, 0, 100), 0, 100);

  Serial.printf("Battery: %.2f V (~%d%%)\n", battMv / 1000.0, percent);
  delay(1000);
}
