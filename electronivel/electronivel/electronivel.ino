int sensorLow = 2;
int sensorHigh = 3;

void setup() {
  pinMode(sensorLow, INPUT_PULLUP);
  pinMode(sensorHigh, INPUT_PULLUP);
  Serial.begin(9600);
}

void loop() {
  bool low = digitalRead(sensorLow) == LOW;
  bool high = digitalRead(sensorHigh) == LOW;

  if (low && !high) {
    Serial.println("BAJO");
  } else if (high) {
    Serial.println("LLENO");
  } else {
    Serial.println("MEDIO");
  }

  delay(1000);
}