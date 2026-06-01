# 🪁 KitSat: GOD Node (Airborne Payload & Transmitter)

This repository contains the firmware for the **GOD Node**, the airborne payload unit of the KitSat (Kite Satellite) telemetry system. It is designed to be flown on a kite to collect atmospheric data, record it locally to a blackbox (SD Card), and transmit the data in real-time to the ground station (Servant Node) via LoRa.

## 🚀 Features

* **High-Speed Telemetry:** Transmits raw C++ structs via LoRa instead of strings/encryption to drastically reduce packet size and latency.
* **Offline Blackbox Logging:** Automatically creates sequential CSV files (e.g., `/DataTerbang_X.csv`) on a Micro SD Card to prevent data overwriting upon sudden mid-air reboots.
* **Smart Duplex Ready:** Constantly listens for incoming commands from the ground station (e.g., Remote File Manager requests) without interrupting the telemetry loop.
* **Crash-Resistant Logic:** Uses non-blocking timers (`millis()`) and state flags to ensure the ESP32 doesn't crash if a sensor or the SD Card is disconnected mid-flight.

## 🛠️ Hardware Requirements

* **Microcontroller:** Wemos Lolin32 Lite (ESP32)
* **Radio:** LoRa Ra-02 SX1278 (433MHz)
* **Environment:** BME280 (Temperature, Humidity, Pressure, Altitude) - *Using default I2C address (0x76)*
* **GPS:** Ublox NEO-7M / M8N
* **Storage:** Standard SPI Micro SD Card Module + FAT32 Micro SD
* **Power:** 3.7V LiPo Battery

## 🔌 Critical Hardware Modifications

**SD Card 3.3V Bypass (AMS1117 Hack):**
Standard Arduino SD Card modules feature an onboard AMS1117 5V-to-3.3V regulator. When powered directly with 3.3V from the Wemos Lolin Lite, the voltage drops to ~2.3V, causing a `(3) The physical drive cannot work` mount error.
* **The Fix:** Solder the 3.3V wire from the ESP32 directly to the large output tab of the AMS1117 chip on the SD module. This bypasses the regulator and delivers clean 3.3V to the memory card.

**SPI Pin Setup:**
Ensure the LoRa and SD Card modules do not conflict on the VSPI bus.
* MOSI: 23, MISO: 19, SCK: 18
* LoRa SS (CS): 5
* SD Card CS: 14

## 📂 Core Data Structure

This node broadcasts the following struct. It must perfectly match the struct on the Servant Node.

```cpp
struct godMessages {
    float latitude;
    float longitude;
    float gpsAltitude;
    float windSpeed;
    float batteryVoltage;
    float temperature;
    float humidity;
    float pressure;
    float bme280Altitude;
    char uptime[9];      // Format HH:MM:SS
};
