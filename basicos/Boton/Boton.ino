/*
  Blink

  Turns an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the UNO, MEGA and ZERO
  it is attached to digital pin 13, on MKR1000 on pin 6. LED_BUILTIN is set to
  the correct LED pin independent of which board is used.
  If you want to know what pin the on-board LED is connected to on your Arduino
  model, check the Technical Specs of your board at:
  https://docs.arduino.cc/hardware/

  modified 8 May 2014
  by Scott Fitzgerald
  modified 2 Sep 2016
  by Arturo Guadalupi
  modified 8 Sep 2016
  by Colby Newman

  This example code is in the public domain.

  https://docs.arduino.cc/built-in-examples/basics/Blink/
*/
const int LED = 4;
const int L2 = 26;
const int L3 = 27;
const int botonPin = 14;  // Pin del botón (puedes cambiarlo)

// the setup function runs once when you press reset or power the board
bool estadoLEDs = false;  // Estado actual de los LEDs
bool estadoBotonAnterior = HIGH;

void setup() {
  pinMode(LED, OUTPUT);
  pinMode(L2, OUTPUT);
  pinMode(L3, OUTPUT);
  pinMode(botonPin, INPUT_PULLUP);  // Resistencia pull-up interna
}

void loop() {
  bool estadoBotonActual = digitalRead(botonPin);
  
  // Detectar flanco descendente (botón presionado)
  if (estadoBotonAnterior == HIGH && estadoBotonActual == LOW) {
    estadoLEDs = !estadoLEDs;  // Cambiar estado
    
    // Aplicar estado a todos los LEDs
    digitalWrite(LED, estadoLEDs);
    digitalWrite(L2, estadoLEDs);
    digitalWrite(L3, estadoLEDs);
    
    delay(50);  // Anti-rebote
  }
  
  estadoBotonAnterior = estadoBotonActual;
}