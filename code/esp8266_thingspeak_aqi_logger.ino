#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

#include <SoftwareSerial.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// =================== WiFi + ThingSpeak ===================
const char* WIFI_SSID = "YOUR_WIFI_NAME";
const char* WIFI_PASS = "YOUR_WIFI_PASSWORD";

String THINGSPEAK_API_KEY = "YOUR_THINGSPEAK_WRITE_API_KEY";
const char* THINGSPEAK_URL = "http://api.thingspeak.com/update";

// =================== OLED ===================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// =================== DHT11 ===================
#define DHTPIN D4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// =================== MQ135 ===================
#define MQ135_PIN A0   // NodeMCU A0 (max 1V input)

// =================== PMS5003 ===================
#define PMS_RX_PIN D5
#define PMS_TX_PIN D6
#define PMS_SET_PIN D7

SoftwareSerial pmsSerial(PMS_RX_PIN, PMS_TX_PIN);

uint16_t pm1_0 = 0, pm2_5 = 0, pm10_0 = 0;

// ============ PMS FRAME READ ============
bool readPMSFrame() {
  if (!pmsSerial.available()) return false;
  if (pmsSerial.read() != 0x42) return false;

  unsigned long startWait = millis();
  while (!pmsSerial.available()) {
    if (millis() - startWait > 500) return false;
  }
  if (pmsSerial.read() != 0x4D) return false;

  uint8_t buffer[30];
  int idx = 0;
  unsigned long startTime = millis();

  while (idx < 30) {
    if (pmsSerial.available()) buffer[idx++] = pmsSerial.read();
    if (millis() - startTime > 1000) return false;
  }

  uint32_t sum = 0x42 + 0x4D;
  for (int i = 0; i < 28; i++) sum += buffer[i];
  uint16_t checksum = ((uint16_t)buffer[28] << 8) | buffer[29];
  if (sum != checksum) return false;

  pm1_0  = ((uint16_t)buffer[8]  << 8) | buffer[9];
  pm2_5  = ((uint16_t)buffer[10] << 8) | buffer[11];
  pm10_0 = ((uint16_t)buffer[12] << 8) | buffer[13];

  return true;
}

// ============ WiFi Connect ============
void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected ✅");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

// ============ Send data to ThingSpeak ============
bool sendToThingSpeak(int mq135, float temp, float hum, uint16_t pm25, uint16_t pm10) {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  WiFiClient client;
  HTTPClient http;

  String url = String(THINGSPEAK_URL) +
               "?api_key=" + THINGSPEAK_API_KEY +
               "&field1=" + String(pm25) +
               "&field2=" + String(pm10) +
               "&field3=" + String(mq135) +
               "&field4=" + String(temp, 1) +
               "&field5=" + String(hum, 0);

  Serial.println("Uploading to ThingSpeak...");
  Serial.println(url);

  http.begin(client, url);
  int httpCode = http.GET();
  http.end();

  if (httpCode > 0) {
    Serial.print("ThingSpeak Response Code: ");
    Serial.println(httpCode);
    return true;
  } else {
    Serial.print("Upload Failed: ");
    Serial.println(httpCode);
    return false;
  }
}

void setup() {
  Serial.begin(9600);
  delay(200);

  // PMS
  pmsSerial.begin(9600);
  pinMode(PMS_SET_PIN, OUTPUT);
  digitalWrite(PMS_SET_PIN, HIGH); // wake PMS

  // DHT
  dht.begin();

  // OLED I2C
  Wire.begin(D2, D1);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Embedded AQI System");
  display.println("Connecting WiFi...");
  display.display();

  connectWiFi();
}

unsigned long lastUpload = 0;
const unsigned long uploadInterval = 20000; // ThingSpeak minimum ~15s (use 20s safe)

void loop() {
  int mq135_raw = analogRead(MQ135_PIN);

  bool pmsOK = false;
  while (pmsSerial.available()) {
    if (readPMSFrame()) {
      pmsOK = true;
      break;
    }
  }

  float temp = dht.readTemperature();
  float hum  = dht.readHumidity();
  bool dhtOK = !(isnan(temp) || isnan(hum));

  // OLED Display
  display.clearDisplay();
  display.setCursor(0, 0);

  display.println("Embedded AQI System");

  if (dhtOK) {
    display.print("T:");
    display.print(temp, 1);
    display.print("C H:");
    display.print(hum, 0);
    display.println("%");
  } else {
    display.println("DHT: Error");
  }

  display.print("MQ135:");
  display.println(mq135_raw);

  if (pmsOK) {
    display.print("PM2.5:");
    display.println(pm2_5);
    display.print("PM10 :");
    display.println(pm10_0);
  } else {
    display.println("PMS: waiting...");
  }

  display.display();

  // Serial logging
  Serial.print("MQ135=");
  Serial.print(mq135_raw);

  if (pmsOK) {
    Serial.print(" | PM2.5=");
    Serial.print(pm2_5);
    Serial.print(" PM10=");
    Serial.print(pm10_0);
  } else {
    Serial.print(" | PMS=WAIT");
  }

  if (dhtOK) {
    Serial.print(" | T=");
    Serial.print(temp);
    Serial.print(" H=");
    Serial.print(hum);
  } else {
    Serial.print(" | DHT=ERR");
  }
  Serial.println();

  // Upload every 20 seconds
  if (millis() - lastUpload > uploadInterval) {
    lastUpload = millis();

    // If PMS not ok, still upload zeros
    uint16_t up_pm25 = pmsOK ? pm2_5 : 0;
    uint16_t up_pm10 = pmsOK ? pm10_0 : 0;
    float up_temp = dhtOK ? temp : 0;
    float up_hum  = dhtOK ? hum : 0;

    sendToThingSpeak(mq135_raw, up_temp, up_hum, up_pm25, up_pm10);
  }
}
