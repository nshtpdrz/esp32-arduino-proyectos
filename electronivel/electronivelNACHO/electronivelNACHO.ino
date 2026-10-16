#include <BluetoothSerial.h>

BluetoothSerial SerialBT;

const int s10 = 15;
const int s8 = 4;
const int s6 = 13;
const int s4 = 14;
const int s2 = 12;

void setup() {
 Serial.begin(115200);
 SerialBT.begin("Electronivel");

 pinMode(s10, INPUT_PULLUP);
 pinMode(s8, INPUT_PULLUP);
 pinMode(s6, INPUT_PULLUP);
 pinMode(s4, INPUT_PULLUP);
 pinMode(s2, INPUT_PULLUP);

 Serial.println("ESP32 Electronivel por BT inicializado");

}

void loop() {
  int r10 = digitalRead(s10);
  int r8 = digitalRead(s8);
  int r6 = digitalRead(s6);
  int r4 = digitalRead(s4);
  int r2 = digitalRead(s2);

  int nivel = 0;

  if(r10 == LOW && r8 == LOW && r6 == LOW && r4 == LOW && r2 == LOW)
  {
    nivel = 100;
  }
  else if(r10 == LOW && r8 == LOW && r6 == LOW && r4 == LOW)
  {
    nivel = 80;

  }else if(r10 == LOW && r8 == LOW && r6 == LOW)
  {
    nivel = 60;

  }else if(r10 == LOW && r8 == LOW)
  {
    nivel = 40;
  }
  else if(r10 == LOW)
  {
    nivel = 20;
  }else
  {
    nivel;
  }

  SerialBT.print(String(nivel));
  Serial.print(String(nivel));

  delay(1000);

}
