#include "IR_remote.h"
#include <Servo.h>

// ========== Modifica los valores IR si tu control no responde ==========
#define KEY_UP    0x18    // Arriba
#define KEY_DOWN  0x52    // Abajo
#define KEY_LEFT  0x08    // Izquierda
#define KEY_RIGHT 0x5A    // Derecha
#define KEY_STAR  0x16    // *
#define KEY_POUND 0x0D    // #
#define KEY_OK    0x1C    // OK
// ================================================================

IRremote ir(12);
Servo servo_A2;

String BT_value, BT_value_temp;
volatile int Left_Tra_Value, Center_Tra_Value, Right_Tra_Value;
volatile int Black = 1;
volatile int Front_Distance, Left_Distance, Right_Distance;
volatile int autoMode = 0;   // 0=Remoto, 1=Evita Obstáculos, 2=Seguidor Línea

// ---------- Control de tiempos para el movimiento ----------
unsigned long actionStartTime = 0;
int actionDuration = 0;          // 0 = movimiento continuo
char currentAction = 0;
int currentSpeed = 0;

// ---------- FUNCIONES DE MOTORES CORREGIDAS PARA TU CARRO ----------
// Según tu mapeo:
// - Derecha debe ir ENFRENTE
// - Izquierda debe ir ATRÁS
// - Arriba debe ir a la IZQUIERDA
// - Abajo debe ir a la DERECHA

void Move_Forward(int car_speed) {
  // Antes era 'Rotate_Right' -> Ahora mapeado a ir ENFRENTE
  digitalWrite(2,LOW); digitalWrite(4,HIGH);
  digitalWrite(7,HIGH); digitalWrite(8,LOW);
  analogWrite(5,car_speed); analogWrite(6,car_speed);
}

void Move_Backward(int car_speed) {
  // Antes era 'Rotate_Left' -> Ahora mapeado a ir ATRÁS
  digitalWrite(2,HIGH); digitalWrite(4,LOW);
  digitalWrite(7,LOW); digitalWrite(8,HIGH);
  analogWrite(5,car_speed); analogWrite(6,car_speed);
}

void Rotate_Left(int car_speed) {
  // Antes era 'Move_Forward' -> Ahora mapeado a girar a la IZQUIERDA
  digitalWrite(2,LOW); digitalWrite(4,HIGH);
  digitalWrite(7,LOW); digitalWrite(8,HIGH);
  analogWrite(5,car_speed); analogWrite(6,car_speed);
}

void Rotate_Right(int car_speed) {
  // Antes era 'Move_Backward' -> Ahora mapeado a girar a la DERECHA
  digitalWrite(2,HIGH); digitalWrite(4,LOW);
  digitalWrite(7,HIGH); digitalWrite(8,LOW);
  analogWrite(5,car_speed); analogWrite(6,car_speed);
}

void STOP() {
  digitalWrite(2,LOW); digitalWrite(4,LOW);
  digitalWrite(7,LOW); digitalWrite(8,LOW);
  analogWrite(5,0); analogWrite(6,0);
}

// ---------- Acción controlada por tiempo (Usada por el IR) ----------
void startAction(char action, int speed_val, int duration) {
  currentAction = action;
  currentSpeed = speed_val;
  actionDuration = duration;
  actionStartTime = millis();

  switch (currentAction) {
    case 'f': Move_Forward(currentSpeed); break;
    case 'b': Move_Backward(currentSpeed); break;
    case 'l': Rotate_Left(currentSpeed); break;
    case 'r': Rotate_Right(currentSpeed); break;
  }
}

// ---------- Función Ultrasónica (Evita Obstáculos) ----------
float checkdistance() {
  digitalWrite(A1, LOW);
  delayMicroseconds(2);
  digitalWrite(A1, HIGH);
  delayMicroseconds(10);
  digitalWrite(A1, LOW);
  float distance = pulseIn(A0, HIGH) / 58.00;
  delay(10);
  return distance;
}

void Ultrasonic_Avoidance() {
  Front_Distance = checkdistance();
  if (Front_Distance <= 20) {
    STOP();
    delay(100);
    servo_A2.write(180);
    delay(500);
    Left_Distance = checkdistance();
    delay(100);
    servo_A2.write(0);
    delay(500);
    Right_Distance = checkdistance();
    delay(100);
    servo_A2.write(90);
    delay(500);
    if (Left_Distance > Right_Distance) {
      Rotate_Left(200);
      delay(500);
    } else {
      Rotate_Right(200);
      delay(500);
    }
  } else {
    Move_Forward(200);
  }
}

// ---------- Función Seguidor de Línea ----------
void Infrared_Tracing() {
  Left_Tra_Value = digitalRead(9);
  Center_Tra_Value = digitalRead(10);
  Right_Tra_Value = digitalRead(11);
  
  if (Center_Tra_Value == Black) {
    if (Left_Tra_Value != Black && Right_Tra_Value != Black) {
      Move_Forward(130);
    } else if (Left_Tra_Value == Black && Right_Tra_Value != Black) {
      Rotate_Left(160);
    } else if (Left_Tra_Value != Black && Right_Tra_Value == Black) {
      Rotate_Right(160);
    }
  } else {
    if (Left_Tra_Value == Black && Right_Tra_Value != Black) {
      Rotate_Left(160);
    } else if (Left_Tra_Value != Black && Right_Tra_Value == Black) {
      Rotate_Right(160);
    } else {
      STOP();
    }
  }
}

void setup() {
  Serial.begin(9600);
  ir.begin();
  
  pinMode(2, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  
  pinMode(9, INPUT);
  pinMode(10, INPUT);
  pinMode(11, INPUT);
  
  servo_A2.attach(A2);
  servo_A2.write(90);
  
  pinMode(A1, OUTPUT); // Trigger
  pinMode(A0, INPUT);  // Echo
}

void loop() {
  // 1. Gestión de tiempos para acciones del IR
  if (actionDuration > 0) {
    if (millis() - actionStartTime >= actionDuration) {
      STOP();
      currentAction = 0;
      actionDuration = 0;
    }
  }

  // 2. Modos automáticos si están activos
  if (autoMode == 1) {
    Ultrasonic_Avoidance();
  } else if (autoMode == 2) {
    Infrared_Tracing();
  }

  // 3. Lectura de Control Remoto Infrarrojo
  if (ir.decode()) {
    byte ir_code = ir.getCode();
    if (ir_code != 0xff) {
      autoMode = 0; 
      switch (ir_code) {
        case KEY_UP:    startAction('f', 255, 300); break; // Ejecuta Move_Forward corregido
        case KEY_DOWN:  startAction('b', 255, 300); break; // Ejecuta Move_Backward corregido
        case KEY_LEFT:  startAction('l', 230, 300); break; // Ejecuta Rotate_Left corregido
        case KEY_RIGHT: startAction('r', 230, 300); break; // Ejecuta Rotate_Right corregido
        case KEY_STAR:  STOP(); autoMode = 1; break;
        case KEY_POUND: STOP(); autoMode = 2; break;
        case KEY_OK:    STOP(); break;
      }
    }
  }

  // 4. Lectura de Bluetooth
  handleBluetooth();
}

void handleBluetooth() {
  while (Serial.available() > 0) {
    char c = Serial.read();
    if (c != '\r' && c != '\n') {
      BT_value_temp += c;
    }
    delay(2);
  }

  if (BT_value_temp.length() > 0) {
    BT_value = BT_value_temp;
    BT_value_temp = ""; 
    BT_value.trim();
  }

  if (BT_value.length() == 0) return;

  if (BT_value == "*") {
    STOP(); currentAction = 0;
    autoMode = (autoMode == 1) ? 0 : 1;
    BT_value = "";
    return;
  }
  if (BT_value == "#") {
    STOP(); currentAction = 0;
    autoMode = (autoMode == 2) ? 0 : 2;
    BT_value = "";
    return;
  }

  char cmd = BT_value[0]; 
  switch (cmd) {
    case 'F': // Arriba -> Va a ejecutar Move_Forward remapeado (Izquierda física anterior)
      autoMode = 0;
      startAction('f', 255, 0); 
      break;   
    case 'B': // Abajo -> Va a ejecutar Move_Backward remapeado (Derecha física anterior)
      autoMode = 0;
      startAction('b', 255, 0); 
      break;
    case 'L': // Izquierda -> Va a ejecutar Rotate_Left remapeado (Atrás físico anterior)
      autoMode = 0;
      startAction('l', 230, 0); 
      break;
    case 'R': // Derecha -> Va a ejecutar Rotate_Right remapeado (Enfrente físico anterior)
      autoMode = 0;
      startAction('r', 230, 0); 
      break;
    case 'S': 
      STOP();
      autoMode = 0;
      currentAction = 0;
      break;
  }
  
  BT_value = ""; 
}