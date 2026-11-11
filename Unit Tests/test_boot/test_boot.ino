// Test to make sure laptop can connect to ESP32D and print to Serial Monitor
// No Wriring required, just connect the ESP32D to your laptop via USB and run this sketch. You should see "Running..." printed every 2 seconds in the Serial Monitor.

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("ESP32D Serial Test OK");
}

void loop() {
  Serial.println("Running...");
  delay(2000);
}