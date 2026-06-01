# 🪁 KitSat (Kite Satellite) Telemetry System

KitSat is an experimental, low-cost embedded telemetry system designed to be flown on a kite. It records atmospheric data, GPS coordinates, and system diagnostics, logging them locally to an SD Card while simultaneously transmitting real-time telemetry to a ground station via LoRa.

The system is divided into two main nodes:
* **GOD (Payload / Transmitter):** The airborne unit responsible for data acquisition (GPS, BME280), SD Card logging, and raw data transmission.
* **Servant (Ground Station / Receiver):** The ground unit that receives telemetry, processes incoming LoRa packets, and displays data via an OLED screen (with upcoming Remote File Manager capabilities).

---

## 🚀 Features

* **Real-time Telemetry:** Uses LoRa (SX1278 433MHz) to transmit data packets reliably over long distances.
* **Raw Struct Transmission:** Telemetry data is sent as raw C++ structs instead of encrypted/string formats to minimize packet size, reduce latency, and prevent signal loss mid-air.
* **Offline Blackbox Logging:** Saves flight data in standard CSV format (`/DataTerbang_X.csv`) to a FAT32 Micro SD Card for post-flight analysis.
* **Remote File Manager (WIP):** Duplex LoRa communication allows the Servant to request file lists or format the GOD's SD Card remotely from the ground.
* **Cross-Platform Ready:** Code is structured for PlatformIO, easily compilable on Linux (Arch/Arch-based environments) and other OS.

---

## 🛠️ Hardware Requirements

### GOD (Airborne Node)
* **Microcontroller:** Wemos Lolin32 Lite (ESP32)
* **LoRa Module:** Ra-02 SX1278 (433MHz)
* **Environment Sensor:** BME280 (Temperature, Humidity, Pressure, Altitude)
* **GPS Module:** NEO-7M / M8N
* **Storage:** Micro SD Card Module (SPI) + 16GB Micro SD (Formatted to FAT32)
* **Power:** 3.7V LiPo Battery

### Servant (Ground Node)
* **Microcontroller:** ESP32 (Any standard dev board)
* **LoRa Module:** Ra-02 SX1278 (433MHz)
* **Display:** 0.96" I2C OLED Display
* **Input:** Push buttons for UI navigation

---

## 🔌 Important Hardware Notes & Quirks

To ensure stability, the following hardware modifications and configurations were implemented:

1.  **BME280 I2C Address:** The BME280 sensor used in this project operates on I2C address `0x60` (not the default `0x76` or `0x77`). Attempting to use standard library defaults will result in initialization failure.
2.  **SD Card 3.3V Bypass (Critical):** Standard Arduino SD Card modules have an onboard AMS1117 (5V to 3.3V) regulator. When powered with 3.3V from the ESP32, the voltage drops to ~2.3V, causing a `(3) The physical drive cannot work` mount error. 
    *Fix:* Solder the 3.3V VCC line directly to the output tab of the AMS1117 chip to bypass the regulator.
3.  **SPI Bus Sharing:** Both LoRa and the SD Card share the same VSPI bus (MOSI: 23, MISO: 19, SCK: 18). Ensure distinct Chip Select (CS) pins are used (LoRa SS: 5, SD CS: 14) and initialize them sequentially.

---

## 💻 Software & Libraries

This project is built using **PlatformIO**. Ensure you do NOT use the legacy Arduino `SD` library. The ESP32's native `FS.h` and `SD.h` are required.

**Core Dependencies:**
* `sandeepmistry/LoRa`
* `mikalhart/TinyGPSPlus`
* `adafruit/Adafruit BME280 Library`
* `adafruit/Adafruit Unified Sensor`

---

## 📂 Data Structure (Telemetry Packet)

The God and Servant nodes communicate by casting this exact `godMessages` struct. Any changes here must be mirrored on both devices.

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
