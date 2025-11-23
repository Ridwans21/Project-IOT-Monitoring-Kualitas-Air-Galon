#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define TDS_PIN 35  // GPIO35 = pin analog input

LiquidCrystal_I2C lcd(0x27, 16, 2);  // alamat 0x27, LCD 16 kolom x 2 baris

void setup() {
  Serial.begin(115200);
  analogReadResolution(12); // Resolusi ADC 12-bit untuk ESP32
  lcd.init();               // Inisialisasi LCD
  lcd.backlight();          // Nyalakan lampu latar LCD
  lcd.setCursor(0, 0);
  lcd.print("Sensor TDS ESP32");
  delay(2000);
  lcd.clear();
}

void loop() {
  const int samples = 10;
  int total = 0;

  // Ambil 10 sample analog dari TDS sensor
  for (int i = 0; i < samples; i++) {
    total += analogRead(TDS_PIN);
    delay(100);  // jeda antar pembacaan
  }

  float avgAnalog = total / float(samples);
  float voltage = avgAnalog * (3.3 / 4095.0);  // Konversi ke tegangan (V)
  
  // Rumus dari datasheet TDS sensor (tanpa kompensasi suhu)
float correctionFactor = 1.396;
float tdsValue = ((133.42 * voltage * voltage * voltage
                 - 255.86 * voltage * voltage
                 + 857.39 * voltage) * 0.5) * correctionFactor;


  // Serial monitor output
  Serial.print("TDS: ");
  Serial.print(tdsValue, 0);
  Serial.print(" ppm | Tegangan: ");
  Serial.print(voltage, 2);
  Serial.println(" V");

  // Tampilkan ke LCD
  lcd.setCursor(0, 0);
  lcd.print("TDS:           ");
  lcd.setCursor(5, 0);
  lcd.print(tdsValue, 0);
  lcd.print(" ppm");

  lcd.setCursor(0, 1);
  lcd.print("V: ");
  lcd.print(voltage, 2);
  lcd.print(" V     "); // padding untuk hapus sisa karakter

  delay(1000);
}
