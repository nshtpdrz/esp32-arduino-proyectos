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
  blink(4000, 0, 1, ledv);
  blink(500, 500, 3, ledv);  
  blink(2000, 0, 1, leda);  
  blink(4000, 0, 1, ledr); 
}

void blink(int time_on, int time_off, int n, int luz)
{
   for(int i=0; i<n; i++)
    {
      digitalWrite(luz, HIGH);
      delay(time_on);	
      
      digitalWrite(luz, LOW);   
      delay(time_off);
    }
}