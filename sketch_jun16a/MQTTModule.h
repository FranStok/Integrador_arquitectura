#ifndef MQTTModule
#define MQTTModule
#include <PubSubClient.h>
#include <Arduino.h> 
#include <WiFi.h>

extern bool silenciarViaMQTT;
extern PubSubClient client;
extern bool isSystemActivated;
extern bool isAlarmOn;
void callback(char* topic, byte* payload, unsigned int length);
void setupClientMQTT();
void procesarComandoMQTT(String topic, String comando);
void connectMQTT();
void publishMascotaPresente(bool isPresent);
void checkSystemStatus();
void publishAgregarMascota(String mascotaInfo);
void publishUpdateSystemStatusInterval(String intervalo);
#endif