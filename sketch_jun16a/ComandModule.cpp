#include <Arduino.h>
#include "inoFunctions.h"
#include "ComandModule.h"
#include "MQTTModule.h"
#include "WIFIModule.h"

void processSetWifi(String comando);

void readCommands() {
  if (!Serial.available()) return;
  String comando = Serial.readString();
  comando.trim();
  String comandoAux=comando; //Para no cambiar las mayusculas del contenido del comando.
  comandoAux.toLowerCase();
  //ejemplo setTag: setTag(nombre,especie,edad)
  if (comandoAux.startsWith("settag(")) {
    if (comandoAux.endsWith(")")) {
      processSetTag(comando.substring(7, comando.length() - 1));
    }
    return;
  }
  //ejemplo setWorkTime: setWorkTime(16:00,21:30)
  if (comandoAux.startsWith("setworktime(")) {
    if (comandoAux.endsWith(")")) {
      processSetWorkTime(comando.substring(12, comando.length() - 1));
    }
    return;
  }
  //ejemplo setwifi: setwifi(ssid,password)
  if (comandoAux.startsWith("setwifi(")) {
    if (comandoAux.endsWith(")")) {
      processSetWifi(comando.substring(8, comando.length() - 1));
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

void processSetWifi(String comando) {
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
  }
  setWifiCredentials(contenido[0],contenido[1]);
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

  if(inicio.hora>23 || inicio.hora<0 || fin.hora>23 || fin.hora<0 ||  fin.minutos>60 || fin.minutos<0 || inicio.minutos>60 || inicio.minutos<0) {
    Serial.println("❌ Los intervalos seleccionados no correponden a un horario valido.");
    return;
  }
  publishUpdateSystemStatusInterval(information[0] + "," + information[1]);
  Serial.println("✅ Intervalo actualizado con exito.");
  delay(1000);  // Evitar múltiples escrituras
}

int splitString(String texto, char delimitador, String resultado[], int maxCount) {
  int count = 0;
  int index = 0;

  while (texto.length() > 0) {
    index = texto.indexOf(delimitador);
    if (count >= maxCount) count++;
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