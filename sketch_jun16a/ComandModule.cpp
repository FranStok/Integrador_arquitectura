#include <Arduino.h>
#include "inoFunctions.h"
#include "ComandModule.h"
#include "MQTTModule.h"

void readCommands() {
  if (!Serial.available()) return;
  String comando = Serial.readString();
  comando.trim();
  comando.toLowerCase();
  //ejemplo setTag: setTag(id,nombre,especie,edad)
  if (comando.startsWith("settag(")) {
    if (comando.endsWith(")")) {
      processSetTag(comando.substring(7, comando.length() - 1));
    }
    return;
  }
  //ejemplo setWorkTime: setWorkTime(16:00,21:30)
  if (comando.startsWith("setworktime(")) {
    if (comando.endsWith(")")) {
      processSetWorkTime(comando.substring(12, comando.length() - 1));
    }
    return;
  }
}

void processSetTag(String comando) {
  String contenido[4];
  int contenidoLength = splitString(comando, ',', contenido, 3);  // 7 = largo de "setTag("
  if (contenidoLength != 3) {
    Serial.println("❌ Se esperaban exactamente 3 campos.");
    return;
  }
  for (int i = 0; i < contenidoLength; i++) {
    String field = contenido[i];
    field.trim();
    if (field.length() == 0) {
      Serial.println("❌ Los campos no pueden estar vacios.");
      return;
    }
  }
  writeInfoToTag(contenido, contenidoLength);
}
void processSetWorkTime(String comando) {
  String contenido[2];
  int contenidoLength = splitString(comando, ',', contenido, 2);
  if (contenidoLength != 2) {
    Serial.println("❌ Se esperaban exactamente 2 campos.");
    return;
  }
  for (int i = 0; i < contenidoLength; i++) {
    String field = contenido[i];
    field.trim();
    if (field.length() == 0) {
      Serial.println("❌ Los campos no pueden estar vacios.");
      return;
    }
    if (field.length() != 5) {
      Serial.println("El formato tiene que ser hh:mm");
      return;
    }
  }
  setTagWorkingTime(contenido);
}

struct Horario StringToHorario(String horarioString) {
  String horarioArray[2];
  splitString(horarioString, ':', horarioArray, 2);
  return Horario{
    horarioArray[0].toInt(),
    horarioArray[1].toInt()
  };
}

void setTagWorkingTime(String information[]) {
  inicio = StringToHorario(information[0]);
  fin = StringToHorario(information[1]);
  publishUpdateSystemStatusInterval(information[0] + "," + information[1]);
  Serial.println("✅ Intervalo actualizado con exito.");
  delay(1000);  // Evitar múltiples escrituras
}

int splitString(String texto, char delimitador, String resultado[], int maxCount) {
  int count = 0;
  int index = 0;

  while (texto.length() > 0) {
    index = texto.indexOf(delimitador);
    if (count >= 4) count++;
    else {
      if (index == -1) {
        resultado[count++] = texto;
        break;
      } else {
        resultado[count++] = texto.substring(0, index);
        texto = texto.substring(index + 1);
      }
    }
  }

  return count;
}