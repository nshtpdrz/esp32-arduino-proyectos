-----------------------------------------------------------------------------------------
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);  // Cambia a 0x3F si no funciona
SoftwareSerial BT(2, 3);            // RX, TX

const int alturaBote = 16;
const int trigPin = 9;
const int echoPin = 10;

void setup() {
  Serial.begin(9600);
  BT.begin(9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  lcd.init();
  lcd.backlight();
}

void loop() {
  long duracion;
  float distancia;

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duracion = pulseIn(echoPin, HIGH);
  distancia = duracion * 0.034 / 2;

  float nivel = alturaBote - distancia;
  if (nivel < 0) nivel = 0;
  if (nivel > alturaBote) nivel = alturaBote;

  float porcentaje = (nivel / alturaBote) * 100;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Nivel: ");
  lcd.print((int)porcentaje);
  lcd.print("%");

  //lcd.setCursor(0, 1);
  //lcd.print("Dist: ");
  //lcd.print(distancia, 1);
  //lcd.print(" cm");

  //Serial.print("Distancia al agua: ");
  //Serial.print(distancia);
  //Serial.print(" cm | Nivel: ");
  Serial.print(porcentaje);
  Serial.println("%");

  // También enviar por Bluetooth
  BT.print("Nivel: ");
  BT.print((int)porcentaje);
  //BT.print("% - Distancia: ");
  //BT.print(distancia, 1);
  //BT.println(" cm");

  delay(1000);
}
