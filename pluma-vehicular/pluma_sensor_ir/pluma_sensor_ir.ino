#include <IRremote.h>
#include <Servo.h>

const byte IR_RECEIVE_PIN = 4;
const int trig = 6;
const int echo = 5;
const int ledRojo = 7;
const int ledVerde = 8;
Servo myServo;
bool plumaArriba = false;

int distancia(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duracion = pulseIn(echoPin, HIGH, 20000); // timeout 20ms
  int d = duracion / 59;
  return d;
}

void setup() {
  Serial.begin(115200);
  Serial.println("Control IR - Semaforo + Servo");

  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);

  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);
  pinMode(ledRojo, OUTPUT);
  pinMode(ledVerde, OUTPUT);

  digitalWrite(ledRojo, LOW);
  digitalWrite(ledVerde, LOW);

  myServo.attach(10);
  myServo.write(0);
}

void loop() {
  int d1 = distancia(trig, echo);

  Serial.print("Sensor: ");
  Serial.print(d1);
  Serial.println(" cm");

  if (d1 < 100) {
    myServo.write(45); // Bajar pluma
    plumaArriba = true;
    digitalWrite(ledVerde, LOW);
    digitalWrite(ledRojo, HIGH);

    do {
      d1 = distancia(trig, echo); // <-- ACTUALIZA la distancia dentro del ciclo

      if (IrReceiver.decode()) {
        int dato = IrReceiver.decodedIRData.command;
        Serial.print("Dato recibido: ");
        Serial.println(dato, HEX);

        digitalWrite(ledRojo, LOW);
        digitalWrite(ledVerde, LOW);

        switch (dato) {
          case 0x18: // Botón 1 → Rojo + Servo 0°
            digitalWrite(ledRojo, HIGH);
            myServo.write(0);
            break;

          case 0x19: // Botón 2 → Amarillo (solo rojo) + Servo 45°
            digitalWrite(ledRojo, HIGH);
            myServo.write(45);
            break;

          case 0x1A: // Botón 3 → Verde + Servo 90°
            digitalWrite(ledVerde, HIGH);
            myServo.write(90);
            break;
        }

        IrReceiver.resume(); // <-- IMPORTANTE
      }

      delay(200); // Pequeño delay para estabilidad del sensor

    } while (d1 < 100); // <-- Se repite mientras haya algo cerca
  }else{
    	myServo.write(0); // Bajar
  	  }
}