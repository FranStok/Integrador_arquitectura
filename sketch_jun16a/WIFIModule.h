#ifndef WIFIModule
#define WIFIModule
#include <Arduino.h> 
#include <WiFi.h>
#include "time.h"

void set_wifi();
String getTime();
struct tm* getTimeStruct();
#endif