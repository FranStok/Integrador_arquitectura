#ifndef ComandModule
#define ComandModule
#include <Arduino.h> 
#include "inoFunctions.h"

extern Horario inicio;
extern Horario fin;
void readCommands();
void processSetTag(String comando);
void processSetWorkTime(String comando);
int splitString(String texto, char delimitador, String resultado[], int maxCount);
void setTagWorkingTime(String information[]);

#endif