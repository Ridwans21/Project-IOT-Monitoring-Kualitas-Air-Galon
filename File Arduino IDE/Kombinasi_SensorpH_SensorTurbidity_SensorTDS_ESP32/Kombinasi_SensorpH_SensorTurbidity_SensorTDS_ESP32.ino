#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);  // LCD 20 kolom 4 baris

#define PH_SENSOR_PIN 34
#define TDS_SENSOR_PIN 35
#define TURBIDITY_SENSOR_PIN 32

float calibration_value = 21.34 + 0.43;
int ph_buffer[10], temp;
float ph_act;

float tdsValue = 0;
float voltageTDS = 0;

float getAverageVoltage(int pin, int samples = 10) {
  float total = 0;
  for (int i = 0; i < samples; i++) {
    total += analogRead(pin);
    delay(10);
  }
  float averageADC = total / samples;
  return averageADC * (3.3 / 4095.0);
}

float getNTU(float voltage) {
  float ntu = -903.23 * voltage + 2980.65;
  if (ntu < 0) ntu = 0;
  if (ntu > 3000) ntu = 3000;
  return ntu;
}

void printFixedLength(float value, int decimalPlaces, int length) {
  char buf[20];
  dtostrf(value, length, decimalPlaces, buf);
  lcd.print(buf);
}

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  analogReadResolution(12);

  lcd.setCursor(0, 0);
  lcd.print("   Sensor Ready   ");
  delay(2000);
  lcd.clear();
}

void loop() {
  bacaPH();
  bacaTDS();
  float voltageTurbidity = getAverageVoltage(TURBIDITY_SENSOR_PIN);
  float ntu = getNTU(voltageTurbidity);

  // Baris 1: pH dan TDS
  lcd.setCursor(0, 0);
  lcd.print("pH: ");
  printFixedLength(ph_act, 2, 5);
  lcd.print(" TDS: ");
  lcd.print((int)tdsValue);
  lcd.print("   "); // clear sisa karakter

  // Baris 2: NTU
  lcd.setCursor(0, 1);
  lcd.print("NTU: ");
  printFixedLength(ntu, 1, 5);
  lcd.print("            ");

  // Baris 3 kosong
  lcd.setCursor(0, 2);
  lcd.print("                    ");

  // Baris 4: Kelayakan tanpa "Status:"
  lcd.setCursor(0, 3);
  if (ph_act >= 6.5 && ph_act <= 8.5 && tdsValue <= 500 && ntu <= 5.0) {
    lcd.print("Layak Minum         ");
  } else {
    lcd.print("Tidak Layak Minum   ");
  }

  // Serial Debug
  Serial.print("pH: ");
  Serial.print(ph_act, 2);
  Serial.print(" | TDS: ");
  Serial.print(tdsValue, 0);
  Serial.print(" ppm | NTU: ");
  Serial.print(ntu, 2);
  Serial.print(" | Vturb: ");
  Serial.println(voltageTurbidity, 2);

  delay(2000);
}

void bacaPH() {
  long avgval = 0;
  for (int i = 0; i < 10; i++) {
    ph_buffer[i] = analogRead(PH_SENSOR_PIN);
    delay(20);
  }

  for (int i = 0; i < 9; i++) {
    for (int j = i + 1; j < 10; j++) {
      if (ph_buffer[i] > ph_buffer[j]) {
        temp = ph_buffer[i];
        ph_buffer[i] = ph_buffer[j];
        ph_buffer[j] = temp;
      }
    }
  }

  for (int i = 2; i < 8; i++) {
    avgval += ph_buffer[i];
  }

  float volt = (float)avgval * 3.3 / 4095.0 / 6.0;
  ph_act = -5.70 * volt + calibration_value;
}

void bacaTDS() {
  const int samples = 10;
  int total = 0;

  for (int i = 0; i < samples; i++) {
    total += analogRead(TDS_SENSOR_PIN);
    delay(30);
  }

  float avgAnalog = total / float(samples);
  voltageTDS = avgAnalog * (3.3 / 4095.0);
  float correctionFactor = 1.396;

  tdsValue = ((133.42 * voltageTDS * voltageTDS * voltageTDS
               - 255.86 * voltageTDS * voltageTDS
               + 857.39 * voltageTDS) * 0.5) * correctionFactor;
}
