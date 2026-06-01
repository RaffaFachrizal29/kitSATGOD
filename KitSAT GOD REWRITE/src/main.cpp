#include <SD.h>
#include <FS.h>
#include <SPI.h>
#include <Arduino.h>
#include <Wire.h>
#include <LoRa.h>
#include <TinyGPS++.h>
#include <Adafruit_BME280.h>
#include "dataStruct.h"

// LORA
#define LORA_SS         5
#define LORA_RST        4
#define LORA_DIO0       2
#define LORA_FREQ       433E6

// GPS
#define GPS_RX 16
#define GPS_TX 17
#define GPS_BAUD 9600

// Battery
#define BATTERY_PIN 35

// BME 280
#define I2C_SCL 22
#define I2C_SDA 15
#define BME_ADDR 0x76

// SD Card
#define SD_CS 14

// Initialization
TinyGPSPlus gps;
HardwareSerial gpsSerial(2);
godMessages godSay;
Adafruit_BME280 bme280;

// Variables Global
float        baselinePressure = 1013.25f;
bool         baselineSet      = false;
String currentLogFileName;

// Callbacks
void sendLoRaPray();
void readGPS();
float readVoltage();
void calibrateBaselineAltitude();
float getRelativeAltitude();
void getUptime();
void SDCardWrite(godMessages data);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  
  // Setup LoRa
  SPI.begin();
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  if (!LoRa.begin(LORA_FREQ)) {
    Serial.print("[E] LoRa Error! :(");
  } else {
    Serial.print("[i] LoRa Active!");
  }
  LoRa.setSpreadingFactor(12);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(5);
  LoRa.setSyncWord(0x12);

  // Setup GPS
  gpsSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX, GPS_TX);
  Serial.printf("\n[GPS] RX=%d TX=%d @%d\n", GPS_RX, GPS_TX, GPS_BAUD);

  // Setup BME 280
  Wire.begin(I2C_SDA, I2C_SCL);
  if (!bme280.begin(BME_ADDR, &Wire)) {
    Serial.println("[E] BME280 Error!");
  } else {
    bme280.setSampling(
      Adafruit_BME280::MODE_NORMAL,
      Adafruit_BME280::SAMPLING_X1,
      Adafruit_BME280::SAMPLING_X1,
      Adafruit_BME280::SAMPLING_X1,
      Adafruit_BME280::FILTER_OFF,
      Adafruit_BME280::STANDBY_MS_500
    );
    delay(300);
    calibrateBaselineAltitude();
    Serial.println("[i] BME280 Active!");
  }

  // Setup SD
  if(!SD.begin(SD_CS)) {
    Serial.println("[E] Error SDCard!");
    godSay.cardStatus = false;
  } else {
    godSay.cardSize = SD.cardSize();
    godSay.cardStatus = true;
    Serial.println("[i] SDCard Active! " + String(godSay.cardSize / 1024));
  }
  for (int i = 1; i <= 1000; i++) {
    currentLogFileName = "/DataTerbang_" + String(i) + ".csv";
    if (!SD.exists(currentLogFileName)) break;
  }
  File godData = SD.open(currentLogFileName, FILE_WRITE);
  if (godData) {
    godData.println("Uptime,Latitude,Longitude,GpsAlt(m),WindSpeed(m/s),Temp(C),Press(hPa),Hum(%),BmeAlt(m),Battery(V)");
    godData.close();
    Serial.println("Current File Log" + currentLogFileName);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  // Mulai GPS
  readGPS();
  getUptime();
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 5000) {
    lastUpdate = millis();
    // Looping stabil
    // Bungkus data Struct GPS
    if (gps.location.isValid()) {
      godSay.longitude = gps.location.lng();
      godSay.latitude = gps.location.lat();
      godSay.gpsAltitude = gps.altitude.meters();
      godSay.windSpeed = gps.speed.mps();
    }
    // Bungkus data struct BME sama Batre loh ya
    godSay.batteryVoltage = readVoltage(); 
    godSay.temperature = bme280.readTemperature();
    godSay.humidity = bme280.readHumidity();
    godSay.pressure = bme280.readPressure() / 100.0, 2;
    godSay.bme280Altitude = getRelativeAltitude();
    SDCardWrite(godSay);
    // Send to LoRa
    sendLoRaPray();
  }
  delay(10);
}

void readGPS() {
  while (gpsSerial.available()) {
    gps.encode(gpsSerial.read());
  }
}

void sendLoRaPray() {
  LoRa.beginPacket();
  LoRa.write((u_int8_t*)&godSay, sizeof(godMessages));
  LoRa.endPacket();
}

float readVoltage() {
  int rawADC = analogRead(BATTERY_PIN);
  float pinVoltage = (rawADC / 4095.0) * 3.3;
  float batteryVoltage = pinVoltage * 2.058;
  return batteryVoltage;
}

void calibrateBaselineAltitude() {
    float sum = 0.0f;
    for (int i = 0; i < 10; i++) {
        sum += bme280.readPressure() / 100.0f;
        delay(50);
    }
    baselinePressure = sum / 10.0f;
    baselineSet = true;
    Serial.printf("[BME] Baseline: %.2f hPa\n", baselinePressure);
}

float getRelativeAltitude() {
    if (!baselineSet) return 0.0f;
    return bme280.readAltitude(baselinePressure);
}

void getUptime() {
  unsigned long totalDetik = millis() / 1000;
  int jam = totalDetik / 3600;
  int menit = (totalDetik % 3600) / 60;
  int detik = totalDetik % 60;

  sprintf(godSay.uptime, "%02d:%02d:%02d", jam, menit, detik);
}

void SDCardWrite(godMessages data) {
  File godData = SD.open(currentLogFileName, FILE_APPEND);
  if (!godData) return;
  godData.printf("%s,%.6f,%.6f,%.1f,%.2f,%.1f,%.1f,%.0f,%0.1f,%.2f\n",
              data.uptime,data.latitude,data.longitude, 
              data.gpsAltitude,data.windSpeed,data.temperature,data.pressure,
              data.humidity,data.bme280Altitude,data.batteryVoltage);
  godData.close();
}