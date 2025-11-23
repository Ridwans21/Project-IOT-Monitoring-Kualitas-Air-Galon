#include <WiFi.h> 
#include <PubSubClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Setup LCD
LiquidCrystal_I2C lcd(0x27, 20, 4);

// Sensor Pins
#define PH_SENSOR_PIN 34
#define TDS_SENSOR_PIN 35
#define TURBIDITY_SENSOR_PIN 32

float calibration_value = 21.34 + 0.43;
int ph_buffer[10], temp;
float ph_act;
float tdsValue = 0;
float voltageTDS = 0;

// Ganti sesuai WiFi & IP broker
const char* ssid = "wifi.id"; 
const char* password = "lulustahunini";
const char* mqtt_server = "192.168.27.244"; 

WiFiClient espClient;
PubSubClient client(espClient);

// Deklarasi fungsi prototype
float getAverageVoltage(int pin, int samples = 10);
float getNTU(float voltage);
void printFixedLength(float value, int decimalPlaces, int length);
void bacaPH();
void bacaTDS();
void setup_wifi();
void reconnect();

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  analogReadResolution(12);

  lcd.setCursor(0, 0);
  lcd.print("   Sensor Ready   ");
  delay(2000);
  lcd.clear();

  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

void setup_wifi() {
  delay(10);
  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32Client")) {
      Serial.println("Connected to MQTT");
      delay(2000);  // Tambahan delay stabilisasi MQTT broker
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  bacaPH();
  bacaTDS();
  float voltageTurbidity = getAverageVoltage(TURBIDITY_SENSOR_PIN);
  float ntu = getNTU(voltageTurbidity);

  lcd.setCursor(0, 0);
  lcd.print("pH: ");
  printFixedLength(ph_act, 2, 5);
  lcd.print(" TDS: ");
  lcd.print((int)tdsValue);
  lcd.print("   ");

  lcd.setCursor(0, 1);
  lcd.print("NTU: ");
  printFixedLength(ntu, 1, 5);
  lcd.print("            ");

  lcd.setCursor(0, 3);
  if (ph_act >= 6.5 && ph_act <= 8.5 && tdsValue <= 500 && ntu <= 5.0) {
    lcd.print("Layak Minum         ");
  } else {
    lcd.print("Tidak Layak Minum   ");
  }

  Serial.print("pH: ");
  Serial.print(ph_act, 2);
  Serial.print(" | TDS: ");
  Serial.print(tdsValue, 0);
  Serial.print(" ppm | NTU: ");
  Serial.print(ntu, 2);
  Serial.print(" | Vturb: ");
  Serial.println(voltageTurbidity, 2);

  // Kirim ke MQTT dalam format JSON
  String payload = "{\"ph\":" + String(ph_act, 2) + ",\"tds\":" + String(tdsValue, 0) + ",\"turbidity\":" + String(ntu, 1) + "}";
  Serial.print("Publish MQTT: ");
  Serial.println(payload);
  
  if (client.publish("sensor/airgalon", payload.c_str(), true)) {
    Serial.println("Publish Success ✅");
  } else {
    Serial.println("Publish Failed ❌");
  }

  delay(3000);
}

float getAverageVoltage(int pin, int samples) {
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
  tdsValue = ((133.42 * voltageTDS * voltageTDS * voltageTDS - 255.86 * voltageTDS * voltageTDS + 857.39 * voltageTDS) * 0.5) * correctionFactor;
}
