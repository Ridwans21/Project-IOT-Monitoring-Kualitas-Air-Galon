void setup() {
  Serial.begin(9600); // Baud rate untuk ESP32, biasanya 115200
}

void loop() {
  int sensorValue = analogRead(32); // Baca dari pin GPIO32
  float voltage = sensorValue * (3.3 / 4095.0); // ESP32: ADC 12-bit (0 - 4095), tegangan referensi 3.3V
  Serial.println(voltage); // Tampilkan hasil tegangan
  delay(500);
}