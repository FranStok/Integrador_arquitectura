#include "MQTTModule.h"
#include "WIFIModule.h"
#include "ComandModule.h"
#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>



const char* broker = "test.mosquitto.org";
const char* topicAlarma = "arqAvanzada/alarma";
const char* topicToggleSystemStatusIntervalDashboard = "arqAvanzada/systemStatus/interval/dashboard";
const char* topicToggleSystemStatusIntervalEsp32 = "arqAvanzada/systemStatus/interval/esp32";
const char* topicToggleSystemStatus = "arqAvanzada/systemStatus/toggle";
const char* topicCheckSystemStatus = "arqAvanzada/SystemStatus/check";
const char* topicAgregarMascota = "arqAvanzada/agregarMascota";
const char* topicInformacionMascota = "arqAvanzada/informacionMascota";
const char* topicPresenciaMascota = "arqAvanzada/presenciaMascota";
const char* topicPresenciaMascotaFecha = "arqAvanzada/presenciaMascotaFecha";
const int mqtt_port = 1883;
WiFiClient espClient;

PubSubClient client(espClient);
//Silenciar boton
bool silenciarViaMQTT = false;

void setupClientMQTT() {
  client.setServer(broker, mqtt_port);
  client.setCallback(callback);
}


void connectMQTT() {
  while (!client.connected()) {
    Serial.println("Intentando conectar a MQTT");

    String clientId = "ESP32Client-FranCAECE";

    if (client.connect(clientId.c_str())) {
      Serial.println("MQTT conectado");
      client.subscribe(topicAlarma);
      client.subscribe(topicInformacionMascota);
      client.subscribe(topicToggleSystemStatus);
      client.subscribe(topicToggleSystemStatusIntervalDashboard);
      checkSystemStatus();
    } else {
      delay(1000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  char messageValue[length + 1];
  for (int i = 0; i < length; i++) {
    messageValue[i] = (char)payload[i];
  }
  messageValue[length] = '\0';  // Importante: terminar el string con '\0'
  String messageStr = String(messageValue);
  String topicStr = String(topic);
  procesarComandoMQTT(topicStr, messageStr);
}

void publishMascotaPresente(bool isPresent) {
  const char* mensaje = isPresent ? "true" : "false";
  client.publish(topicPresenciaMascota, mensaje);
  if (isPresent) client.publish(topicPresenciaMascotaFecha, getTime().c_str());
}

void procesarComandoMQTT(String topic, String comando) {
  if (topic == topicAlarma) {
    if (comando == "silenciar") {
      if (isAlarmOn) silenciarViaMQTT = true;
      return;
    }
  }
  if (topic == topicInformacionMascota && !comando.isEmpty()) {
    processSetTag(comando);
    return;
  }
  if (topic == topicToggleSystemStatus) {
    isSystemActivated = comando == "false" ? false : true;
    return;
  }
  if (topic == topicToggleSystemStatusIntervalDashboard) {
    String contenido[2];
    int contenidoLength = splitString(comando, ',', contenido, 2);
    setTagWorkingTime(contenido);
    return;
  }
}

void publishAgregarMascota(String mascotaInfo){
  client.publish(topicAgregarMascota, mascotaInfo.c_str());
  delay(500);
}
void publishUpdateSystemStatusInterval(String intervalo){
  client.publish(topicToggleSystemStatusIntervalEsp32, intervalo.c_str());
  delay(500);
}

void checkSystemStatus() {
  client.publish(topicCheckSystemStatus, "");
  delay(500);
}
