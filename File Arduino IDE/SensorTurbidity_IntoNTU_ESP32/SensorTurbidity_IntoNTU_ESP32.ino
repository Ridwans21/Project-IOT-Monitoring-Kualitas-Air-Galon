#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
const int sensorPin = 32;

// Fungsi untuk merata-ratakan pembacaan ADC
float getAverageVoltage(int pin, int samples = 10) {
  float total = 0;
  for (int i = 0; i < samples; i++) {
    total += analogRead(pin);
    delay(10);
  }
  float averageADC = total / samples;
  return averageADC * (3.3 / 4095.0);
}

// Fungsi print float fixed length ke LCD, agar sisa karakter lama hilang
void printFixedLength(float value, int decimalPlaces, int length) {
  char buf[16];
  dtostrf(value, length, decimalPlaces, buf);
  lcd.print(buf);
}

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Turbidity Meter");
  delay(1000);
}

void loop() {
  float voltage = getAverageVoltage(sensorPin);

  // Rumus linear kalibrasi manual
  float ntu = -903.23 * voltage + 2980.65;

  if (ntu < 0) ntu = 0;
  if (ntu > 3000) ntu = 3000;

  Serial.print("V: ");
  Serial.print(voltage, 2);
  Serial.print(" V | NTU: ");
  Serial.println(ntu, 2);

  // Bersihkan seluruh baris pertama
  lcd.setCursor(0, 0);
  lcd.print("                ");  // 16 spasi untuk clear baris 1
  // Tulis baris pertama baru
  lcd.setCursor(0, 0);
  lcd.print("V: ");
  printFixedLength(voltage, 2, 5);
  lcd.print(" V");

  // Bersihkan seluruh baris kedua
  lcd.setCursor(0, 1);
  lcd.print("                ");  // 16 spasi untuk clear baris 2
  // Tulis baris kedua baru
  lcd.setCursor(0, 1);
  lcd.print("NTU: ");
  printFixedLength(ntu, 2, 6);

  delay(500);
}