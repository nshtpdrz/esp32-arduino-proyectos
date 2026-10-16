#include <BluetoothSerial.h>

BluetoothSerial SerialBT;

// Pines de electrodos
const int PIN_CRIT  = 13;  // Nivel 1 (crítico)
const int PIN_BAJO  = 12;  // Nivel 2 (bajo)
const int PIN_MEDIO = 14;  // Nivel 3 (medio)
const int PIN_ALTO  = 27;  // Nivel 4 (alto)
const int PIN_LLENO = 26;  // Nivel 5 (lleno)

void setup() {
  // Inicializa puertos serie
  Serial.begin(115200);
  SerialBT.begin("ElectroNivel"); // Nombre que verá tu celular

  // Configura los pines de entrada con pull‑down interno
  pinMode(PIN_CRIT,  INPUT_PULLDOWN);
  pinMode(PIN_BAJO,  INPUT_PULLDOWN);
  pinMode(PIN_MEDIO, INPUT_PULLDOWN);
  pinMode(PIN_ALTO,  INPUT_PULLDOWN);
  pinMode(PIN_LLENO, INPUT_PULLDOWN);

  Serial.println("ESP32 ElectroNivel por BT inicializado");
}

void loop() {
  // Lee el nivel: devuelve 5 si lleno, 4 alto, …, 1 crítico, 0 sin agua
  int nivel = 0;
  if (digitalRead(PIN_LLENO) == HIGH)  nivel = 5;
  else if (digitalRead(PIN_ALTO)  == HIGH)  nivel = 4;
  else if (digitalRead(PIN_MEDIO) == HIGH) nivel = 3;
  else if (digitalRead(PIN_BAJO)  == HIGH)  nivel = 2;
  else if (digitalRead(PIN_CRIT)  == HIGH)  nivel = 1;

  // Envía por Bluetooth y por USB
  SerialBT.println(nivel);
  Serial.println("Nivel: " + String(nivel));

  delay(1000);  // espera 1 segundo antes de la siguiente lectura
}