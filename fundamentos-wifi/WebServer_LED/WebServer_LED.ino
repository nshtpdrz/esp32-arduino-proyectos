#include <WiFi.h>
#include <WebServer.h>
#include "secrets.h"   // copia secrets.example.h como secrets.h

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

WebServer server(80);
int led = 5;

void handleRoot() {
  server.send(200, "text/html",
  "<h1>Control LED</h1><a href='/on'>Encender</a><br><a href='/off'>Apagar</a>");
}

void handleOn() {
  digitalWrite(led, HIGH);
  server.send(200, "text/plain", "LED encendido");
}

void handleOff() {
  digitalWrite(led, LOW);
  server.send(200, "text/plain", "LED apagado");
}

void setup() {
  pinMode(led, OUTPUT);
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(200);
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/on", handleOn);
  server.on("/off", handleOff);
  server.begin();
}

void loop() { 
  server.handleClient(); 
}
