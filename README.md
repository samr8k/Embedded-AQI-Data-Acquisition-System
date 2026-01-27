

# 🌫️ IoT-Based Air Quality Monitoring & Control System

An **embedded, state-driven air quality monitoring system** using **ESP8266**, capable of real-time AQI calculation, sensor fault detection, noise-filtered data processing, OLED visualization, and cloud telemetry via ThingSpeak.

This project is designed as an **industry-style proof of concept (PoC)** for environmental monitoring applications.

---

## 📌 Features

*  **Real-time PM monitoring** (PM1.0, PM2.5, PM10) using PMS5003
*  **Gas sensing** using MQ135
*  **State-based system architecture**

  * INIT (sensor warm-up)
  * GOOD
  * MODERATE
  * UNHEALTHY
  * HAZARDOUS
  * FAULT
*  **Sensor warm-up handling (INIT state)** to avoid unstable readings
*  **Noise reduction using Exponential Moving Average (EMA) filtering**
*  **Sensor fault detection**
*  **Live OLED display**
*  **Cloud logging & visualization using ThingSpeak**
*  **Rate-limited cloud updates** (15-second interval)

---

## 🧠 System Architecture

```
Sensors → Filtering → AQI Calculation → State Machine
   ↓            ↓              ↓            ↓
 PMS5003     EMA Filter      AQI Index   System State
 MQ135                                          ↓
                                          OLED + Cloud
```

All outputs (OLED display and cloud telemetry) are driven by a **single system state**, ensuring consistency and reliability.

---

## 🛠️ Hardware Used

| Component             | Description                     |
| --------------------- | ------------------------------- |
| ESP8266               | Main microcontroller with Wi-Fi |
| PMS5003               | Laser particulate matter sensor |
| MQ135                 | Gas sensor                      |
| OLED (SSD1306)        | 128×64 display                  |
| 10kΩ Resistors        | Pull-up / voltage divider       |
| External Power Supply | Stable 5V recommended           |

---

## 📦 Software & Libraries

* **Arduino IDE**
* **ESP8266 Board Package**
* Libraries:

  * `ESP8266WiFi`
  * `ThingSpeak`
  * `Adafruit_GFX`
  * `Adafruit_SSD1306`
  * `SoftwareSerial`

---

## ⚙️ How It Works

### 1️⃣ INIT / Warm-up State

* System ignores sensor data for the first **60 seconds**
* Prevents unstable readings at startup
* OLED displays *“System INIT – Warming sensors”*

### 2️⃣ Sensor Acquisition

* PMS5003 data read using a **non-blocking serial parser**
* MQ135 read via ADC

### 3️⃣ Noise Reduction

* PM2.5 values filtered using **Exponential Moving Average (EMA)**

  ```text
  PM_filtered = α × PM_current + (1 − α) × PM_previous
  ```

### 4️⃣ AQI Calculation

* AQI computed using EPA-style breakpoints based on PM2.5

### 5️⃣ State Decision

* AQI and fault flags mapped to a **system state**
* Faults override AQI-based states

### 6️⃣ Output Handling

* OLED displays AQI and current system state
* ThingSpeak logs:

  * PM1, PM2.5, PM10
  * AQI
  * Gas level
  * System state code

---

## 📊 ThingSpeak Field Mapping

| Field   | Description       |
| ------- | ----------------- |
| Field 1 | PM1.0             |
| Field 2 | PM2.5             |
| Field 3 | PM10              |
| Field 4 | AQI               |
| Field 5 | MQ135 Raw Value   |
| Field 6 | System State Code |

---

## ⚠️ Fault Detection Logic

The system enters `STATE_FAULT` if:

* PMS5003 data is unavailable
* PM values exceed valid range
* Sensor communication fails

This ensures **fail-safe behavior**, a key industry requirement.

---

## 🚀 Applications

* Indoor air quality monitoring
* Smart buildings
* Industrial environmental monitoring
* EV cabin air quality systems (PoC)
* IoT-based pollution analytics

---



## 🔮 Future Improvements

* Automatic ventilation / fan control
* MQTT-based communication
* CAN-based sensor network (EV use-case)
* Data analytics & prediction
* Mobile dashboard

---

## 👨‍💻 Author

**Samrat**

Mechanical Engineering (EV & Electronics)

Interest: Embedded Systems, IoT, EV Technologies

