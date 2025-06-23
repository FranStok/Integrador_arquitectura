#ifndef inoFunctions
#define inoFunctions

#include <Arduino.h>

// 📅 Estructura para horarios
struct Horario {
  int hora;
  int minutos;
};

bool writeInfoToTag(String information[], int informationLength);
void setTagWorkingTime(String information[], int informationLength);

#endif