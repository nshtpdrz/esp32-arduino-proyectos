#include <BluetoothSerial.h>

// Verificar si Bluetooth está disponible
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth no está habilitado! Por favor habilítalo en la configuración del Arduino IDE.
#endif

BluetoothSerial SerialBT;

// Nuevo pin para sensor de agua
const int PIN_CRIT  = 12;  // Nivel 1 (crítico)
const int PIN_BAJO  = 13;  // Nivel 2 (bajo)
const int PIN_MEDIO = 14;  // Nivel 3 (medio)
const int PIN_ALTO  = 15;  // Nivel 4 (alto)
const int PIN_LLENO = 16;  // Nivel 5 (lleno)
const int PIN_POWER = 17;  // Pin para alimentar los sensores (evita electrólisis)

// Variables para control de tiempo
unsigned long previousMillis = 0;
const long interval = 1000;  // Intervalo de lectura (1 segundo)

// Variables para detección de nivel con filtro
int nivelActual = 0;
int lecturasPrevias[5] = {0, 0, 0, 0, 0};
int indiceLectura = 0;

void setup() {
  // Inicializa puertos serie
  Serial.begin(115200);
  SerialBT.begin("ElectroNivel_V2"); // Nombre actualizado

  // Configura los pines de entrada con resistencias pull-down
  pinMode(PIN_CRIT, INPUT_PULLDOWN);
  pinMode(PIN_BAJO, INPUT_PULLDOWN);
  pinMode(PIN_MEDIO, INPUT_PULLDOWN);
  pinMode(PIN_ALTO, INPUT_PULLDOWN);
  pinMode(PIN_LLENO, INPUT_PULLDOWN);
  
  // Configura el pin de alimentación como salida
  pinMode(PIN_POWER, OUTPUT);
  digitalWrite(PIN_POWER, LOW);  // Inicialmente apagado

  Serial.println("ESP32 ElectroNivel V2 inicializado");
  
  // Prueba inicial de LEDs y sensores
  verificarSensores();
}

void loop() {
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    
    // Activa la energía para los sensores solo durante la lectura
    digitalWrite(PIN_POWER, HIGH);
    delay(50);  // Pequeña pausa para estabilización
    
    // Lee el nivel actual
    int nivelLeido = leerNivel();
    
    // Aplica filtro para evitar lecturas erróneas (almacena en buffer circular)
    lecturasPrevias[indiceLectura] = nivelLeido;
    indiceLectura = (indiceLectura + 1) % 5;
    
    // Determina el nivel más común en las últimas 5 lecturas
    nivelActual = nivelMasComun();
    
    // Apaga la energía para evitar electrólisis
    digitalWrite(PIN_POWER, LOW);
    
    // Envía por Bluetooth y por USB
    SerialBT.println(nivelActual);
    Serial.println("Nivel: " + String(nivelActual));
    
    // Imprime información de diagnóstico
    imprimirEstadoSensores();
  }
}

int leerNivel() {
  // Lee el nivel: devuelve 5 si lleno, 4 alto, ..., 1 crítico, 0 sin agua
  int nivel = 0;
  
  if (digitalRead(PIN_LLENO) == HIGH)  nivel = 5;
  else if (digitalRead(PIN_ALTO)  == HIGH)  nivel = 4;
  else if (digitalRead(PIN_MEDIO) == HIGH)  nivel = 3;
  else if (digitalRead(PIN_BAJO)  == HIGH)  nivel = 2;
  else if (digitalRead(PIN_CRIT)  == HIGH)  nivel = 1;
  
  return nivel;
}

int nivelMasComun() {
  int conteo[6] = {0, 0, 0, 0, 0, 0};  // Contadores para niveles 0-5
  
  // Contar ocurrencias de cada nivel
  for (int i = 0; i < 5; i++) {
    conteo[lecturasPrevias[i]]++;
  }
  
  // Encontrar el nivel más común
  int nivelMasComun = 0;
  int maxConteo = 0;
  
  for (int i = 0; i < 6; i++) {
    if (conteo[i] > maxConteo) {
      maxConteo = conteo[i];
      nivelMasComun = i;
    }
  }
  
  return nivelMasComun;
}

void verificarSensores() {
  Serial.println("Verificando sensores...");
  
  digitalWrite(PIN_POWER, HIGH);
  delay(100);
  
  Serial.print("Sensor Crítico (PIN " + String(PIN_CRIT) + "): ");
  Serial.println(digitalRead(PIN_CRIT) == HIGH ? "ACTIVO" : "inactivo");
  
  Serial.print("Sensor Bajo (PIN " + String(PIN_BAJO) + "): ");
  Serial.println(digitalRead(PIN_BAJO) == HIGH ? "ACTIVO" : "inactivo");
  
  Serial.print("Sensor Medio (PIN " + String(PIN_MEDIO) + "): ");
  Serial.println(digitalRead(PIN_MEDIO) == HIGH ? "ACTIVO" : "inactivo");
  
  Serial.print("Sensor Alto (PIN " + String(PIN_ALTO) + "): ");
  Serial.println(digitalRead(PIN_ALTO) == HIGH ? "ACTIVO" : "inactivo");
  
  Serial.print("Sensor Lleno (PIN " + String(PIN_LLENO) + "): ");
  Serial.println(digitalRead(PIN_LLENO) == HIGH ? "ACTIVO" : "inactivo");
  
  digitalWrite(PIN_POWER, LOW);
  Serial.println("Verificación completada");
}

void imprimirEstadoSensores() {
  Serial.println("Estado actual de los sensores:");
  Serial.print("CRIT: ");
  Serial.print(digitalRead(PIN_CRIT));
  Serial.print(" | BAJO: ");
  Serial.print(digitalRead(PIN_BAJO));
  Serial.print(" | MEDIO: ");
  Serial.print(digitalRead(PIN_MEDIO));
  Serial.print(" | ALTO: ");
  Serial.print(digitalRead(PIN_ALTO));
  Serial.print(" | LLENO: ");
  Serial.println(digitalRead(PIN_LLENO));
}