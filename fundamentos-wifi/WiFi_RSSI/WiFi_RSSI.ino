#include <WiFi.h>
#include "secrets.h"   // copia secrets.example.h como secrets.h

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Conectando");
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }

  Serial.println("\nConectado!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void loop() {

  Serial.print("RSSI: ");
  Serial.println(WiFi.RSSI());
  delay(1000);

}
