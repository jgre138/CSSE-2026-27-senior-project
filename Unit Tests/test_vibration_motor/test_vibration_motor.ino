// TEST: Vibration motor - turns on for 1 second, off for 1 second
// Wiring: GPIO 25 -> ~1k resistor -> transistor base (2N2222 or similar), emitter -> GND,
//         motor between 5V and the transistor collector, flyback diode across the motor.
// Don't connect the motor directly to a GPIO pin.

constexpr uint8_t MOTOR_PIN = 25;

void setup() {
  Serial.begin(115200);
  pinMode(MOTOR_PIN, OUTPUT);
}

void loop() {
  Serial.println("Motor ON");
  digitalWrite(MOTOR_PIN, HIGH);
  delay(1000);

  Serial.println("Motor OFF");
  digitalWrite(MOTOR_PIN, LOW);
  delay(1000);
}
