// C++ code
//
int ledr = 27;
int leda = 32;
int ledv = 14;

void setup()
{
  pinMode(ledr, OUTPUT);
  pinMode(leda, OUTPUT);
  pinMode(ledv, OUTPUT);
  
}

void loop()
{
  // Verde encendido
  digitalWrite(ledv, HIGH);
  delay(4000);
  digitalWrite(ledv, LOW);
  
  // Amarillo encendido
  digitalWrite(leda, HIGH);
  delay(2000);
  digitalWrite(leda, LOW);
  
  // Rojo encendido
  digitalWrite(ledr, HIGH);
  delay(4000);
  digitalWrite(ledr, LOW);
}
