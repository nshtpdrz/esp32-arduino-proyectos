#include <WiFi.h>
#include <WebServer.h>
#include "secrets.h"   // copia secrets.example.h como secrets.h

// 1. Credenciales WiFi
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

// 2. Servidor HTTP
WebServer server(80);

// 3. Pines de los 6 LEDs
const int ledPins[] = {23, 22, 21, 19, 18, 5};
const int numLeds = 6;

int sliderValue = 0;

// --- HTML dinámico ---
String getHTML() {
  String html = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>body{font-family: Arial; text-align: center; margin-top: 50px;}";
  html += ".slider {width: 80%; margin: 20px;} h1 {color: #333;}</style>";
  html += "</head><body>";
  html += "<h1>Control de LEDs ESP32</h1>";
  html += "<p>Mueve el slider para encender los LEDs (0 a 6)</p>";
  html += "<input type='range' min='0' max='6' value='" + String(sliderValue) + "' class='slider' onchange='updateSlider(this.value)'>";
  html += "<p>Valor actual: <strong>" + String(sliderValue) + "</strong></p>";
  html += "<script>function updateSlider(val) { window.location.href = '/rangeInput' + val; }</script>";
  html += "</body></html>";
  return html;
}

// Página principal
void handleRoot() {
  server.send(200, "text/html", getHTML());
}

void handleNotFound() {
  server.send(404, "text/plain", "404: Not Found");
}

// Actualizar LEDs según slider
void updateLeds(int value) {
  sliderValue = value;

  for (int i = 0; i < numLeds; i++) {
    digitalWrite(ledPins[i], i < value ? HIGH : LOW);
  }

  server.send(200, "text/html", getHTML());
}

// Rutas del slider
void handleRange0() { updateLeds(0); }
void handleRange1() { updateLeds(1); }
void handleRange2() { updateLeds(2); }
void handleRange3() { updateLeds(3); }
void handleRange4() { updateLeds(4); }
void handleRange5() { updateLeds(5); }
void handleRange6() { updateLeds(6); }

void setup() {
  Serial.begin(115200);

  for (int i = 0; i < numLeds; i++) {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW);
  }

  Serial.println("Conectando a WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Conectado! IP: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/rangeInput0", handleRange0);
  server.on("/rangeInput1", handleRange1);
  server.on("/rangeInput2", handleRange2);
  server.on("/rangeInput3", handleRange3);
  server.on("/rangeInput4", handleRange4);
  server.on("/rangeInput5", handleRange5);
  server.on("/rangeInput6", handleRange6);

  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("Servidor iniciado");
}

void loop() {
  server.handleClient();
}
