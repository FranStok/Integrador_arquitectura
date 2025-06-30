#include <WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <MFRC522.h>
#include "MQTTModule.h"
#include "WIFIModule.h"
#include "ComandModule.h"
#include "inoFunctions.h"

//Buzzer
const int buzzer = 32;
const int led = 22;
const int boton = 15;
const int soundTime = 5000;


//Horarios
Horario inicio = { 23, 0 };
Horario fin = { 8, 30 };


//RFID
const int RSA_PIN = 5;
const int RST_PIN = 27;
const int bloquesUtilizados[] = { 4, 5, 6 };
int bloquesUtilizadosLength = sizeof(bloquesUtilizados) / sizeof(bloquesUtilizados[0]);
MFRC522 rfid(RSA_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

//Silenciar boton
int botonPresionado = LOW;
bool isAlarmOn = false;

bool isSystemActivated = true;

String registeredIds[20];
int registeredIdsLength = 0;


void RFID_setup() {
  SPI.begin();
  rfid.PCD_Init();
  delay(100);
  rfid.PCD_DumpVersionToSerial();  //Muestra en el Serial.
  // Clave por defecto
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;
}

void setup() {
  pinMode(buzzer, OUTPUT);
  pinMode(led, OUTPUT);
  pinMode(boton, INPUT_PULLUP);
  digitalWrite(buzzer, HIGH);  // Aseguramos que el buzzer arranque apagado

  Serial.begin(9600);
  //Conexiones
  checkWifiAndTryReconect();
  delay(500);
  //RFID
  RFID_setup();
}

// Autenticar el bloque
bool autenticar_bloque(int bloque) {
  MFRC522::StatusCode status;
  status = rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, bloque, &key, &(rfid.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Error de autenticación: ");
    Serial.println(rfid.GetStatusCodeName(status));
    return false;
  }
  return true;
}

String getTagId() {
  //Leo el ID del tag.
  String uidString;
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) uidString += "0";  // padding
    uidString += String(rfid.uid.uidByte[i], HEX);
  }
  return uidString;
}

// Leer el bloque
bool leer_bloque(int bloque) {
  byte buffer[18];  // 16 bytes + 2 de control
  byte size = sizeof(buffer);

  MFRC522::StatusCode status;

  status = rfid.MIFARE_Read(bloque, buffer, &size);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Error al leer: ");
    Serial.println(rfid.GetStatusCodeName(status));
    return false;
  }

  //Leo el nombre de la mascota guardado.
  String mascota;
  for (uint8_t i = 0; i < 16; i++) {
    if (buffer[i] > 0) {
      mascota += (char)buffer[i];
    }
  }
  Serial.print("Datos del bloque ");
  Serial.print(bloque);
  Serial.print(": ");
  Serial.println(mascota);
  return true;
}

int isIdRegistered(String id) {
  id.toLowerCase();
  for (int i = 0; i < registeredIdsLength; i++) {
    String currentId = registeredIds[i];
    currentId.toLowerCase();
    if (id == currentId) {
      return i;
    }
  }
  return -1;
}

void RFID_loop() {
  int sector = -1;
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return;
  }
  String uidString = getTagId();
  uidString.toUpperCase();
  Serial.println("TAG UID: " + uidString);
  if (isIdRegistered(uidString) == -1) return;
  for (int i = 0; i < bloquesUtilizadosLength; i++) {
    int auxSector = bloquesUtilizados[i] / 4;  //Tengo que autorizar una vez por sector.

    if (auxSector != sector) {
      sector = auxSector;
      if (!autenticar_bloque(bloquesUtilizados[i])) {
        haltRfid();
        return;
      };
    }
    if (!leer_bloque(bloquesUtilizados[i])) {
      haltRfid();
      return;
    }
  }



  digitalWrite(buzzer, LOW);
  digitalWrite(led, HIGH);
  publishMascotaPresente(true);
  unsigned long startTime = millis();
  isAlarmOn = true;
  //Controlo que suene por 5 segundos. Si se silencia por via MQTT, entonces debemos poner la variable en falso de vuelta.
  while (millis() - startTime < soundTime) {
    client.loop();
    if (digitalRead(boton) == HIGH || silenciarViaMQTT || !isSystemActivated) {
      break;  // Se presionó el botón, cancelamos
    }
    delay(10);  // pequeña espera para evitar lecturas excesivas
  }
  silenciarViaMQTT = false;
  isAlarmOn = false;
  digitalWrite(buzzer, HIGH);
  digitalWrite(led, LOW);
  publishMascotaPresente(false);

  haltRfid();
  delay(1000);
}

bool escribir_bloque(int bloque, String field) {
  byte dataBlock[16];

  // Limpiar el array
  memset(dataBlock, 0, 16);

  // Copiar los primeros 16 caracteres del String
  for (int i = 0; i < 16 && i < field.length(); i++) {
    dataBlock[i] = field[i];  // char -> byte
  }
  MFRC522::StatusCode status;
  // Escribir el bloque
  status = rfid.MIFARE_Write(bloque, dataBlock, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Error al escribir: ");
    Serial.println(rfid.GetStatusCodeName(status));
    return false;
  }
  return true;
}

void haltRfid() {
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

bool writeInfoToTag(String information[], int informationLength) {
  int sector = -1;  //Tengo que autorizar una vez por sector.
  int intentos = 0;
  Serial.println("Acerque el TAG");
  while (intentos < 100 && (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial())) {
    intentos++;
    delay(100);
  }
  if (intentos == 100) {
    Serial.println("No acerco ningun TAG.");
    return false;
  }
  String tagId = getTagId();
  String mascotaInfo = tagId + ",";
  for (int i = 0; i < informationLength; i++) {
    int auxSector = bloquesUtilizados[i] / 4;  //Tengo que autorizar una vez por sector.

    String field = information[i];

    mascotaInfo = mascotaInfo + field + ((i == informationLength - 1) ? "" : ",");
    if (field.length() > 16 || field.length() == 0) {
      Serial.println("Campo muy largo o vacio. Debe ser menor a 17 caracteres.");
      haltRfid();
      return false;
    };


    if (sector != auxSector) {
      sector = auxSector;
      if (!autenticar_bloque(bloquesUtilizados[i])) {
        haltRfid();
        return false;
      }
    }


    if (!escribir_bloque(bloquesUtilizados[i], field)) {
      haltRfid();
      return false;
    }
  }
  if (isIdRegistered(tagId) == -1) {
    registeredIds[registeredIdsLength++] = tagId;
  };
  Serial.println("✅ Información escrita en la tarjeta con éxito.");
  publishAgregarMascota(mascotaInfo);
  haltRfid();
  return true;
}

bool isOnWorkingTime() {
  struct tm* now = getTimeStruct();
  if (now == nullptr) return false;
  int minutosActuales = now->tm_hour * 60 + now->tm_min;
  int minutosInicio = inicio.hora * 60 + inicio.minutos;
  int minutosFin = fin.hora * 60 + fin.minutos;
  if (minutosInicio < minutosFin) {
    // Caso normal, por ejemplo: 09:00 a 17:00
    return minutosActuales >= minutosInicio && minutosActuales <= minutosFin;
  } else {
    // Cruza la medianoche, por ejemplo: 23:00 a 08:00
    return minutosActuales >= minutosInicio || minutosActuales <= minutosFin;
  }
}

void loop() {
  checkWifiAndTryReconect();
  if (!client.connected()) connectMQTT();
  client.loop();
  if (isSystemActivated) {
    readCommands();
    if (!isOnWorkingTime()) {
      return;
    }
    RFID_loop();
  }
}