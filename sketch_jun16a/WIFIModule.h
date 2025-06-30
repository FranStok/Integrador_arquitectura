#ifndef WIFIModule
#define WIFIModule
#include <Arduino.h> 
#include <WiFi.h>
#include "time.h"

bool set_wifi();
String getTime();
void setWifiCredentials(String ssid, String password);
struct tm* getTimeStruct();
void checkWifiAndTryReconect();
#endif