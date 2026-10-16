#include <SPI.h>
#include <MFRC522.h>


// Pines de conexión RC522 con ESP32 (ajusta si usas otros)
#define SS_PIN    5    // SDA o SS
#define RST_PIN   21   // RST


MFRC522 mfrc522(SS_PIN, RST_PIN);
String inputData = "";


// 🔹 Prototipo de la función
bool esperarYCargarTarjeta(unsigned long timeout = 15000);


void setup() {
  Serial.begin(115200);
  while (!Serial);


  Serial.println("Inicializando RC522...");
  // SPI en ESP32: SCK=18, MISO=19, MOSI=23, SS=5
  SPI.begin(18, 19, 23, SS_PIN);
  mfrc522.PCD_Init();


  Serial.println("RFID RC522 listo para usar");
}


void loop() {
  Serial.println("\n===== MENU RFID =====");
  Serial.println("1. Escribir en tarjeta");
  Serial.println("2. Leer tarjeta");
  Serial.println("3. Mostrar UID y tipo");
  Serial.print("Seleccione opción: ");


  while (Serial.available() == 0);
  char opcion = Serial.read();
  while (Serial.available()) Serial.read(); // Limpia el buffer


  switch (opcion) {
    case '1': escribirDatos(); break;
    case '2': leerDatos(); break;
    case '3': mostrarUIDyTipo(); break;
    default:
      Serial.println("Opción inválida.");
      delay(2000);
  }
}


// === Función: Espera y detecta una tarjeta ===
bool esperarYCargarTarjeta(unsigned long timeout) {
  unsigned long start = millis();
  Serial.println("Acerca la tarjeta al lector...");
  while (millis() - start < timeout) {
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
      return true;
    }
    delay(50);
  }
  Serial.println("Timeout: No se detectó tarjeta.");
  return false;
}


// === Mostrar UID y tipo de tarjeta ===
void mostrarUIDyTipo() {
  if (!esperarYCargarTarjeta()) return;


  Serial.print("UID: ");
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
  }
  Serial.println();


  MFRC522::PICC_Type tipo = mfrc522.PICC_GetType(mfrc522.uid.sak);
  Serial.print("Tipo de tarjeta: ");
  Serial.println(mfrc522.PICC_GetTypeName(tipo));


  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  delay(3000);
}


// === Escribir datos en bloque 1 ===
void escribirDatos() {
  Serial.println("\nIngrese información (max 15 caracteres, finalice con '#'):");
  inputData = "";
  bool finalizado = false;


  while (!finalizado) {
    if (Serial.available()) {
      char c = Serial.read();
      if (c == '#') finalizado = true;
      else if (inputData.length() < 15) inputData += c;
    }
  }


  Serial.print("Datos a escribir: ");
  Serial.println(inputData);


  if (!esperarYCargarTarjeta()) return;


  byte block = 1; // ⚠️ Bloque 1 (seguro). NO modifiques bloque 0.
  byte buffer[16];
  memset(buffer, 0, 16);
  for (int i = 0; i < inputData.length(); i++) buffer[i] = inputData[i];


  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;


  MFRC522::StatusCode status = mfrc522.PCD_Authenticate(
    MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid)
  );


  if (status != MFRC522::STATUS_OK) {
    Serial.print("Error en autenticación: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }


  status = mfrc522.MIFARE_Write(block, buffer, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Error escribiendo: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
  } else {
    Serial.println("✅ Datos escritos con éxito!");
  }


  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  delay(3000);
}


// === Leer datos del bloque 1 ===
void leerDatos() {
  if (!esperarYCargarTarjeta()) return;


  byte block = 1;
  byte buffer[18];
  byte size = sizeof(buffer);


  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;


  MFRC522::StatusCode status = mfrc522.PCD_Authenticate(
    MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid)
  );


  if (status != MFRC522::STATUS_OK) {
    Serial.print("Error en autenticación: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }


  status = mfrc522.MIFARE_Read(block, buffer, &size);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Error leyendo: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
  } else {
    Serial.print("Datos leídos: ");
    for (byte i = 0; i < 16; i++) Serial.print((char)buffer[i]);
    Serial.println();
  }


  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  delay(3000);
}


/* ⚠️ NOTAS IMPORTANTES DE SEGURIDAD ⚠️
---------------------------------------------------
1️⃣ No escribas en el bloque 0: contiene el UID y datos del fabricante.
   Si lo sobrescribes, podrías "brickear" (dañar permanentemente)
   la tarjeta y hacerla ilegible.


2️⃣ Cada sector tiene un "sector trailer" (último bloque del sector)
   donde están las claves A/B y los bits de acceso. No lo modifiques
   salvo que sepas cómo recalcular los bits de control.


3️⃣ Usa solo bloques de datos (por ejemplo bloque 1, 2, 4...).
   El tamaño total típico de MIFARE Classic 1K son 64 bloques (0–63).


4️⃣ Puedes leer el UID y tipo sin autenticación, pero cualquier
   intento de leer o escribir datos requiere autenticar con la clave.


5️⃣ Si cambias las claves (por ejemplo las 0xFF 0xFF ...), anótalas:
   sin ellas, no podrás volver a leer ni escribir ese sector.
---------------------------------------------------
*/
