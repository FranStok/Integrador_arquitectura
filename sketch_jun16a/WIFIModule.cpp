#include <Arduino.h>
#include <WiFi.h>
#include <Preferences.h>
#include "WIFIModule.h"
#include "MQTTModule.h"
#include "time.h"
#include "ComandModule.h"

Preferences preferences;
//Conexiones
// const char* ssid = "Fibertel WiFi133 2.4GHz";
// const char* password = "01424914297";
String ssid = "";
String password = "";

// Servidor NTP y zona horaria
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -3 * 3600;  // Argentina = GMT-3
const int daylightOffset_sec = 0;

bool wifiIsSet = false;

bool set_wifi() {
  preferences.begin("wifi", true);
  ssid = preferences.getString("ssid", "");
  password = preferences.getString("password", "");
  preferences.end();
  if (ssid.length() == 0 || password.length() == 0) {
    Serial.println("Ingresar credenciales red WIFI.");
    return false;
  }
  WiFi.begin(ssid.c_str(), password.c_str());
  Serial.print("conectando");
  int intentos = 0;
  while (WiFi.status() != WL_CONNECTED) {
    intentos++;
    if ((intentos % 10) == 0) Serial.print(".");
    if (intentos == 100) {
      Serial.println("");
      Serial.println("Conexión fallida. Intentando nuevamente. Pruebe cambiando red WIFI");
      return false;
    }
    delay(100);
  };
  Serial.println("");

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("conectado");
    wifiIsSet = true;
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    setupClientMQTT();
    connectMQTT();
    return true;
  }
  return false;
}

void setWifiCredentials(String ssid, String password) {
  preferences.begin("wifi", false);
  preferences.putString("ssid", ssid);
  preferences.putString("password", password);
  wifiIsSet = false;
  unsigned long tiempoInicioIntentoWifi = 0;
  set_wifi();
  while (!wifiIsSet) {
    tiempoInicioIntentoWifi = millis();
    ///Espero 10 segundos para reintentar de vuelta.
    while (millis() - tiempoInicioIntentoWifi < 10000) {
      readCommands();
    }
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