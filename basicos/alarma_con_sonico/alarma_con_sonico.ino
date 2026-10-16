const int trigPin = 5;
const int echoPin = 18;
const int ledVerde = 14;
const int ledAmarillo = 12;
const int ledRojo = 13;
const int buzzer = 27;

long duracion;
int distancia;

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(ledVerde, OUTPUT);
  pinMode(ledAmarillo, OUTPUT);
  pinMode(ledRojo, OUTPUT);
  pinMode(buzzer, OUTPUT);
  Serial.begin(115200);
}

void loop() {
  // Generar pulso ultrasónico
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Calcular duración y distancia
  duracion = pulseIn(echoPin, HIGH);
  distancia = duracion * 0.034 / 2;
  
  Serial.print("Distancia: ");
  Serial.print(distancia);
  Serial.println(" cm");
  
  if (distancia > 30) { 
    digitalWrite(ledVerde, HIGH);
    digitalWrite(ledAmarillo, LOW);
    digitalWrite(ledRojo, LOW);
    noTone(buzzer);
  } 
  else if (distancia <= 30 && distancia > 15) { 
    digitalWrite(ledVerde, LOW);
    digitalWrite(ledAmarillo, HIGH);
    digitalWrite(ledRojo, LOW);
    tone(buzzer, 1000, 200);
    delay(200);
  } 
  else { 
    digitalWrite(ledVerde, LOW);
    digitalWrite(ledAmarillo, LOW);
    digitalWrite(ledRojo, HIGH);
    tone(buzzer, 2000);
  }
  
  delay(300);
}