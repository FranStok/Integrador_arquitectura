#include <Arduino.h>
#include <WiFi.h>
#include "WIFIModule.h"
#include "time.h"

//Conexiones
const char* ssid = "Fibertel WiFi133 2.4GHz";
const char* password = "01424914297";

// Servidor NTP y zona horaria
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -3 * 3600;  // Argentina = GMT-3
const int daylightOffset_sec = 0;


void set_wifi() {
  WiFi.begin(ssid, password);
  Serial.println("conectando");
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
  };
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("conectado");
  }
}

String getTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return "";
  char timeString[30];
  strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(timeString);
}
struct tm* getTimeStruct() {
  static struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return nullptr;
  return &timeinfo;
}