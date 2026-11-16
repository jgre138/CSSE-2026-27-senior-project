// Test to make sure laptop can connect to ESP32D and print to Serial Monitor
// No Wriring required, just connect the ESP32D to your laptop via USB and run this sketch. You should see "Running..." printed every 2 seconds in the Serial Monitor.
// Make sure that the Serial Monitor is set to the correct baud rate (115200) and the correct COM port for your ESP32D.
// Also the Flash Mode should be set to "DIO" in the Arduino IDE under Tools > Flash Mode.

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("ESP32D Serial Test OK");
}

void loop() {
  Serial.println("Running...");
  delay(2000);
}