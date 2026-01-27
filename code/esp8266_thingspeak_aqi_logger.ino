#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ThingSpeak.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>

// ---------- WiFi ----------
const char* ssid = "botX";
const char* password = "qwertyuiop98765";

// ---------- ThingSpeak ----------
WiFiClient client;
unsigned long channelID = 3239920;
const char* writeAPIKey = "PGMQWWDFRVW9914D";

// ---------- OLED ----------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ---------- MQ135 ----------
#define MQ135_PIN A0

// ---------- PMS5003 ----------
SoftwareSerial pmsSerial(D6, D7); // RX, TX
uint16_t pm1 = 0, pm25 = 0, pm10 = 0;

// ---------- AQI ----------
int calcAQI_PM25(float pm) {
  float C = pm;
  int aqi;

  if (C <= 12.0) aqi = (50.0 / 12.0) * C;
  else if (C <= 35.4) aqi = ((100 - 51) / (35.4 - 12.1)) * (C - 12.1) + 51;
  else if (C <= 55.4) aqi = ((150 - 101) / (55.4 - 35.5)) * (C - 35.5) + 101;
  else if (C <= 150.4) aqi = ((200 - 151) / (150.4 - 55.5)) * (C - 55.5) + 151;
  else if (C <= 250.4) aqi = ((300 - 201) / (250.4 - 150.5)) * (C - 150.5) + 201;
  else if (C <= 500.4) aqi = ((500 - 301) / (500.4 - 250.5)) * (C - 250.5) + 301;
  else aqi = 500;

  return aqi;
}

String aqiCategory(int aqi) {
  if (aqi <= 50) return "Good";
  if (aqi <= 100) return "Moderate";
  if (aqi <= 150) return "USG";
  if (aqi <= 200) return "Unhealthy";
  if (aqi <= 300) return "V.Unhealthy";
  return "Hazardous";
}

// ---------- System State ----------
typedef enum {
  STATE_INIT = 0,
  STATE_GOOD,
  STATE_MODERATE,
  STATE_UNHEALTHY,
  STATE_HAZARDOUS,
  STATE_FAULT
} system_state_t;

system_state_t currentState = STATE_INIT;

// ---------- Filtering ----------
float pm25Filtered = 0;
const float alpha = 0.3;   // EMA smoothing factor



// ✅ Non-blocking safer PMS read
bool readPMS() {
  while (pmsSerial.available()) {
    if (pmsSerial.peek() == 0x42) {
      if (pmsSerial.available() < 32) return false;

      uint8_t start1 = pmsSerial.read();
      uint8_t start2 = pmsSerial.read();
      if (start1 != 0x42 || start2 != 0x4D) continue;

      uint8_t buffer[30];
      buffer[0] = start1;
      buffer[1] = start2;

      for (int i = 2; i < 30; i++) buffer[i] = pmsSerial.read();

      pm1  = (buffer[10] << 8) | buffer[11];
      pm25 = (buffer[12] << 8) | buffer[13];
      pm10 = (buffer[14] << 8) | buffer[15];

      return true;
    } else {
      pmsSerial.read();
    }
  }
  return false;
}

system_state_t getSystemState(int aqi, bool fault) {

  if (fault) return STATE_FAULT;

  if (aqi <= 50) return STATE_GOOD;
  if (aqi <= 100) return STATE_MODERATE;
  if (aqi <= 200) return STATE_UNHEALTHY;
  return STATE_HAZARDOUS;
}


unsigned long lastSend = 0;

void setup() {
  Serial.begin(9600);
  pmsSerial.begin(9600);

  // OLED
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // WiFi
  WiFi.begin(ssid, password);
  display.setCursor(0, 0);
  display.println("Connecting WiFi...");
  display.display();

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected!");
  Serial.println(WiFi.localIP());

  ThingSpeak.begin(client);

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("WiFi Connected!");
  display.print("IP: ");
  display.println(WiFi.localIP());
  display.display();
}

void loop() {

  // ---------- INIT / Warm-up ----------
  static unsigned long bootTime = 0;
  if (bootTime == 0) {
    bootTime = millis();
  }

  if (millis() - bootTime < 60000) {   // 60 sec warm-up
    currentState = STATE_INIT;

    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("System INIT");
    display.println("Warming sensors");
    display.display();

    delay(500);
    return;   // exit loop during warm-up
  }

  // ---------- Sensor Read ----------
  bool sensorFault = false;

  bool pmsOk = readPMS();
  if (!pmsOk) {
    sensorFault = true;
  }

  if (pm25 < 0 || pm25 > 1000) sensorFault = true;
  if (pm10 < 0 || pm10 > 1000) sensorFault = true;

  int gasRaw = analogRead(MQ135_PIN);

  // ---------- Filtering + AQI ----------
  pm25Filtered = alpha * pm25 + (1 - alpha) * pm25Filtered;
  int aqi = calcAQI_PM25(pm25Filtered);

  // ---------- State Update ----------
  currentState = getSystemState(aqi, sensorFault);

  // ---------- OLED ----------
  display.clearDisplay();
  display.setCursor(0, 0);

  display.print("PM2.5: ");
  display.println(pm25);

  display.print("AQI: ");
  display.println(aqi);

  display.print("State: ");
  switch (currentState) {
    case STATE_GOOD: display.println("GOOD"); break;
    case STATE_MODERATE: display.println("MODERATE"); break;
    case STATE_UNHEALTHY: display.println("UNHEALTHY"); break;
    case STATE_HAZARDOUS: display.println("HAZARDOUS"); break;
    case STATE_FAULT: display.println("FAULT"); break;
    case STATE_INIT: display.println("INIT"); break;
  }

  display.display();

  // ---------- ThingSpeak ----------
  if (millis() - lastSend > 15000) {
    lastSend = millis();

    ThingSpeak.setField(1, pm1);
    ThingSpeak.setField(2, pm25);
    ThingSpeak.setField(3, pm10);
    ThingSpeak.setField(4, aqi);
    ThingSpeak.setField(5, gasRaw);
    ThingSpeak.setField(6, currentState);

    int status = ThingSpeak.writeFields(channelID, writeAPIKey);
    Serial.print("ThingSpeak status: ");
    Serial.println(status);
  }

  delay(300);
}
