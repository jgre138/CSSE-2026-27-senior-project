// TEST: Buzzer - turns on for 1 second, off for 1 second
// Wiring: Buzzer + -> GPIO 23, Buzzer - -> GND 

constexpr uint8_t BUZZER_PIN = 23; // This pin is at GIOP23. You can change it to any other GPIO pin that is available for your specific board.

void setup() {
  Serial.begin(115200);
  pinMode(BUZZER_PIN, OUTPUT);
}

void loop() {
  Serial.println("Buzzer ON");
  tone(BUZZER_PIN, 2000); //Passive buzzer
  //digitalWrite(BUZZER_PIN, HIGH); // Active buzzer 
  delay(1000);

  Serial.println("Buzzer OFF");
  noTone(BUZZER_PIN);
  //digitalWrite(BUZZER_PIN, LOW);
  delay(1000);
}

//This code does work, BUT the buzzer itself is not loud enough to be heard. 
