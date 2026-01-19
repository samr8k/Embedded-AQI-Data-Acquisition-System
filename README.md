# Embedded AQI Data Acquisition System 🌍📊

The **Embedded AQI Data Acquisition System** is a multi-sensor air quality monitoring prototype that provides **real-time monitoring**, **data logging**, and **graph-based visualization** for analysis.  
It integrates particulate sensing, gas trend monitoring, and environmental sensing, making it a complete embedded solution for indoor/outdoor air quality monitoring.

This project is designed to be extendable into a full **IoT + AI-integrated smart air monitoring system** in future.

---

## ✨ Key Features

✅ **Real-Time Air Quality Monitoring**  
- PM1.0 / PM2.5 / PM10 measurement using **PMS5003**
- Gas/Air quality trend detection using **MQ135**
- Temperature & Humidity measurement using **DHT11**

✅ **Live Display Output (OLED)**  
- Real-time values displayed on **0.96" OLED SSD1306 (I2C)**

✅ **Data Logging & Analysis Ready**  
- Continuous data logging in structured format (CSV/Serial/Cloud-ready)
- Supports analysis and visualization through graphs

✅ **Graph Visualization (Trends)**  
- PM2.5 vs Time
- PM10 vs Time
- MQ135 vs Time
- Temperature/Humidity vs Time

✅ **AQI Severity Indication (RGB LED)**  
- Quick visual indicator of air quality level:
  - Good / Moderate / Poor / Severe

✅ **Regulated Power Supply Design**  
- Uses **LM2596 Buck Converter** for stable 5V regulation

---

## 🧠 System Overview

The system collects data from all sensors, processes it on the microcontroller, displays it on the OLED in real time, logs the data for later analysis, and indicates AQI severity using RGB LED.

---

## 🧩 Block Diagram

<img width="1536" height="1024" alt="aqi" src="https://github.com/user-attachments/assets/55403b90-1409-44fe-8ab6-3c2ff7a2d68e" />


---

## 🔌 Connection Diagram

![WhatsApp Image 2026-01-19 at 8 49 21 PM](https://github.com/user-attachments/assets/9119b373-b319-4d85-b9ed-16380d352ca5)


---

## 🏗️ Prototype (Hardware Build)

> Final hardware is assembled on a **perfboard (no breadboard)** for stable and reliable operation.



## 🧰 Hardware Components

| Component | Purpose |
|----------|---------|
| Arduino Nano / NodeMCU ESP8266 | Main controller |
| PMS5003 | PM1.0 / PM2.5 / PM10 sensing |
| MQ135 | Gas / Air quality trend sensing |
| DHT11 | Temperature & Humidity sensing |
| OLED SSD1306 (0.96") | Live data display |
| RGB LED | AQI severity indication |
| LM2596 Buck Converter | Stable regulated 5V supply |

---

## 📌 Working Principle (Summary)

1. **PMS5003** measures PM concentration via UART communication  
2. **MQ135** provides analog air quality trend output  
3. **DHT11** gives temperature & humidity context  
4. Microcontroller processes and formats the sensor values  
5. OLED shows live readings  
6. RGB LED indicates severity based on PM2.5 thresholds  
7. Data is logged for graph plotting and trend analysis




## 🚀 Future Scope (AI + IoT Integration)

This project can be upgraded into an intelligent smart system by adding:

- ✅ Cloud dashboard (ThingSpeak / Firebase / MQTT)
- ✅ AI-based pollution prediction
- ✅ Anomaly detection for sudden pollution spikes
- ✅ Health recommendations based on AQI level
- ✅ Smart ventilation control automation



## 📍 Applications

- Indoor air quality monitoring (rooms, labs, classrooms)
- Pollution trend study in local environments
- Smart home air monitoring + alerts
- Environmental data collection system for research




