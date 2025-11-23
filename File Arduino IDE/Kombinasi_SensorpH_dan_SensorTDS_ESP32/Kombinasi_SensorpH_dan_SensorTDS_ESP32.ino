#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

// Pin sensor
#define PH_SENSOR_PIN 34
#define TDS_SENSOR_PIN 35

// Sensor pH
float calibration_value = 21.34 + 0.43;
int ph_buffer[10], temp;
float ph_act;

// Sensor TDS
float tdsValue = 0;
float voltageTDS = 0;

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  analogReadResolution(12);

  lcd.setCursor(0, 0);
  lcd.print("pH & TDS Ready");
  delay(2000);
  lcd.clear();
}

void loop() {
  bacaPH();
  delay(500); // Jeda antar sensor
  bacaTDS();

  // Tampilkan hasil ke LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("pH:");
  lcd.print(ph_act, 2);
  lcd.print(" TDS:");
  lcd.print(tdsValue, 0);

  lcd.setCursor(0, 1);
  if (ph_act >= 6.5 && ph_act <= 8.5 && tdsValue < 300) {
    lcd.print("Aman untuk diminum");
  } else {
    lcd.print("Tidak layak minum");
  }

  // Serial monitor
  Serial.print("pH: ");
  Serial.print(ph_act, 2);
  Serial.print(" | TDS: ");
  Serial.print(tdsValue, 0);
  Serial.print(" ppm | V: ");
  Serial.println(voltageTDS, 2);

  delay(3000);
}

void bacaPH() {
  long avgval = 0;

  for (int i = 0; i < 10; i++) {
    ph_buffer[i] = analogRead(PH_SENSOR_PIN);
    delay(20);
  }

  // Sort data
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