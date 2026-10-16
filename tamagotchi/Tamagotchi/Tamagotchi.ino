#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h> // Para guardar los datos permanentemente

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET     -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- PINES ---
#define BTN_A  12  // Navegar Menú / Botón Izquierdo en Juegos
#define BTN_B  13  // Confirmar Acción / Botón Derecho en Juegos
#define BTN_C  14  // Estadísticas / Volver / Reset
#define BUZZER 33  

// --- TAMAÑO DE LA EEPROM ---
#define EEPROM_SIZE 32

// --- ESTADOS DE LA MASCOTA ---
int hambre = 20;      
int felicidad = 80;   
int salud = 100;      
int energia = 100;     
int monedas = 5;       
int diasVividos = 0;
int edadCiclos = 0;
int contadorNegligencia = 0; // Cuenta cuántas veces se enfermó o acumuló popó

bool tienePopo = false;
bool estaDormido = false;
bool luzPrendida = true;
int tipoEvolucion = 0; // 0: Bebé, 1: Normal, 2: Ángel, 3: Monstruo

// --- NAVEGACIÓN ---
int opcionMenu = 0;   
const int TOTAL_OPCIONES = 5; // Añadimos la opción del Casino
const char* nombresMenu[] = { "ALIMENTAR", "JUGAR", "CASINO", "CURAR", "ACCION ESP." };

// ESTADOS DE PANTALLA
bool mostrandoStatus = false;
int modoJuegoActivo = 0; // 0: Normal, 1: Adivina Lado, 2: Tragamonedas Casino, 3: Menú Borrar Partida

// VARIABLES DE MINIJUEGOS
int rondaActual = 0, puntosJugador = 0;
int slot1 = 0, slot2 = 0, slot3 = 0;

// TEMPORIZADORES
unsigned long ultimoCicloVida = 0;
unsigned long cicloVidaIntervalo = 6000; 
unsigned long ultimaAnimacion = 0;
int frameAnimacion = 0;

// --- AUDIO ---
void sonarPitido(int frec, int dur) { tone(BUZZER, frec, dur); }
void sonarTriste() { tone(BUZZER, 220, 150); delay(150); tone(BUZZER, 150, 300); }
void sonarFeliz() { tone(BUZZER, 900, 80); delay(80); tone(BUZZER, 1300, 120); }
void sonarJackpot() { for(int i=0; i<3; i++) { tone(BUZZER, 1000, 100); delay(100); tone(BUZZER, 1500, 100); delay(100); } }

// --- FUNCIONES DE ALMACENAMIENTO (EEPROM) ---
void guardarPartida() {
  EEPROM.write(0, hambre);
  EEPROM.write(1, felicidad);
  EEPROM.write(2, salud);
  EEPROM.write(3, energia);
  EEPROM.write(4, monedas);
  EEPROM.write(5, diasVividos);
  EEPROM.write(6, tipoEvolucion);
  EEPROM.write(7, contadorNegligencia);
  EEPROM.commit(); // Obligatorio en ESP32 para confirmar el guardado
}

void cargarPartida() {
  // Si la EEPROM está vacía (por primera vez devuelve 255), inicializa valores por defecto
  if (EEPROM.read(2) == 255 || EEPROM.read(2) == 0) {
    resetearMascota();
    return;
  }
  hambre = EEPROM.read(0);
  felicidad = EEPROM.read(1);
  salud = EEPROM.read(2);
  energia = EEPROM.read(3);
  monedas = EEPROM.read(4);
  diasVividos = EEPROM.read(5);
  tipoEvolucion = EEPROM.read(6);
  contadorNegligencia = EEPROM.read(7);
}

void resetearMascota() {
  hambre = 20; felicidad = 80; salud = 100; energia = 100;
  monedas = 5; diasVividos = 0; edadCiclos = 0; contadorNegligencia = 0;
  tienePopo = false; estaDormido = false; luzPrendida = true; tipoEvolucion = 0;
  guardarPartida();
}

// --- SETUP ---
void setup() {
  pinMode(BTN_A, INPUT_PULLUP);
  pinMode(BTN_B, INPUT_PULLUP);
  pinMode(BTN_C, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);

  Wire.begin(27, 26);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  
  EEPROM.begin(EEPROM_SIZE);
  cargarPartida(); // Recuperar los datos de la memoria flash al encender
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(12, 25);
  display.println(F("TAMAGOTCHI OS V4"));
  display.display();
  sonarFeliz();
  delay(1500);
}

// --- LÓGICA DE MINIJUEGOS ---
void jugarAdivinaLado(int eleccionJugador) {
  int ladoBicho = random(0, 2);
  display.clearDisplay();
  display.setCursor(15, 15); display.print(F("Tu: ")); display.print(eleccionJugador == 0 ? F("IZQ") : F("DER"));
  display.setCursor(15, 30); display.print(F("Bicho: ")); display.print(ladoBicho == 0 ? F("IZQ") : F("DER"));
  
  display.drawRoundRect(80, 15, 30, 24, 4, SSD1306_WHITE);
  if (ladoBicho == 0) { display.fillRect(83, 22, 3, 4, SSD1306_WHITE); display.fillRect(93, 22, 3, 4, SSD1306_WHITE); }
  else { display.fillRect(91, 22, 3, 4, SSD1306_WHITE); display.fillRect(101, 22, 3, 4, SSD1306_WHITE); }
  
  if (eleccionJugador == ladoBicho) { puntosJugador++; display.setCursor(25, 52); display.print(F("¡BIEN!")); sonarPitido(1000, 60); }
  else { display.setCursor(25, 52); display.print(F("¡FALLO!")); sonarPitido(300, 80); }
  display.display();
  delay(1200);
  
  rondaActual++;
  if (rondaActual >= 3) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(20, 25);
    if (puntosJugador >= 2) { display.print(F("¡GANASTE! +4 Monedas")); monedas += 4; felicidad = min(100, felicidad + 25); sonarFeliz(); }
    else { display.print(F("PERDISTE... :( ")); felicidad = max(0, felicidad - 15); sonarTriste(); }
    display.display();
    delay(2000);
    modoJuegoActivo = 0; guardarPartida();
  }
}

void ejecutarCasino() {
  // Animación de slots girando
  for (int i = 0; i < 6; i++) {
    display.clearDisplay();
    display.drawRect(20, 20, 24, 24, SSD1306_WHITE); display.drawRect(52, 20, 24, 24, SSD1306_WHITE); display.drawRect(84, 20, 24, 24, SSD1306_WHITE);
    display.setCursor(29, 28); display.print(random(1, 4));
    display.setCursor(61, 28); display.print(random(1, 4));
    display.setCursor(93, 28); display.print(random(1, 4));
    display.display();
    sonarPitido(700 + (i*50), 40);
    delay(150);
  }
  
  slot1 = random(1, 4); slot2 = random(1, 4); slot3 = random(1, 4);
  display.clearDisplay();
  display.drawRect(20, 20, 24, 24, SSD1306_WHITE); display.drawRect(52, 20, 24, 24, SSD1306_WHITE); display.drawRect(84, 20, 24, 24, SSD1306_WHITE);
  display.setCursor(29, 28); display.print(slot1);
  display.setCursor(61, 28); display.print(slot2);
  display.setCursor(93, 28); display.print(slot3);
  
  display.setCursor(15, 54);
  if (slot1 == slot2 && slot2 == slot3) {
    display.print(F("JACKPOT! +15 Monedas"));
    monedas += 15; felicidad = min(100, felicidad + 35);
    display.display(); sonarJackpot();
  } else if (slot1 == slot2 || slot2 == slot3 || slot1 == slot3) {
    display.print(F("PREMIO RECTO! +3 M."));
    monedas += 3; display.display(); sonarFeliz();
  } else {
    display.print(F("NADA... SUERTE"));
    display.display(); sonarTriste();
  }
  delay(2500);
  modoJuegoActivo = 0; guardarPartida();
}

// --- LOOP PRINCIPAL ---
void loop() {
  unsigned long tiempoActual = millis();

  // --- MODO JUEGO 1: ADIVINA EL LADO ---
  if (modoJuegoActivo == 1) {
    display.clearDisplay();
    display.setCursor(12, 4); display.print(F("ADIVINA: Ronda ")); display.print(rondaActual + 1);
    display.setCursor(22, 18); display.print(F("[¿A donde ve?]"));
    display.setCursor(4, 54); display.print(F("[A] IZQ        [B] DER"));
    display.display();
    if (digitalRead(BTN_A) == LOW) { jugarAdivinaLado(0); delay(200); }
    if (digitalRead(BTN_B) == LOW) { jugarAdivinaLado(1); delay(200); }
    return;
  }

  // --- MODO JUEGO 3: MENÚ RESET/BORRAR PARTIDA ---
  if (modoJuegoActivo == 3) {
    display.clearDisplay();
    display.setCursor(8, 6); display.print(F("¿RESETEAR MASCOTA?"));
    display.setCursor(10, 22); display.print(F("Borrara todo el"));
    display.setCursor(10, 34); display.print(F("progreso guardado"));
    display.setCursor(2, 54); display.print(F("[A] BORRAR    [C] ATRAS"));
    display.display();
    if (digitalRead(BTN_A) == LOW) { resetearMascota(); modoJuegoActivo = 0; sonarTriste(); delay(400); }
    if (digitalRead(BTN_C) == LOW) { modoJuegoActivo = 0; sonarPitido(600, 50); delay(200); }
    return;
  }

  // --- MOTOR DEL TIEMPO ---
  if (tiempoActual - ultimoCicloVida > cicloVidaIntervalo) {
    ultimoCicloVida = tiempoActual;
    if (salud > 0) {
      edadCiclos++;
      if (edadCiclos % 12 == 0) {
        diasVividos++;
        guardarPartida(); // Guarda de forma segura cada vez que pasa un día virtual
      }
      
      // EVALUAR ÁRBOL GENÉTICO (Al día 3)
      if (diasVividos >= 3 && tipoEvolucion == 0) {
        if (contadorNegligencia <= 1 && salud > 80) tipoEvolucion = 2;      // ÁNGEL
        else if (contadorNegligencia >= 5 || salud < 40) tipoEvolucion = 3; // MONSTRUO
        else tipoEvolucion = 1;                                             // NORMAL
        sonarJackpot();
      }

      // Ciclo de Sueño espontáneo
      if (edadCiclos % 30 == 0) { estaDormido = !estaDormido; if(estaDormido) sonarTriste(); else { sonarFeliz(); luzPrendida = true; } }

      if (!estaDormido) {
        hambre = min(100, hambre + 5); felicidad = max(0, felicidad - 5); energia = max(0, energia - 4);
        if (!tienePopo && hambre < 50 && random(0, 5) == 0) { tienePopo = true; contadorNegligencia++; sonarTriste(); }
      } else {
        if (luzPrendida) { salud = max(0, salud - 10); energia = max(0, energia - 6); contadorNegligencia++; }
        else { salud = min(100, salud + 4); energia = min(100, energia + 12); }
      }

      if (tienePopo) { felicidad = max(0, felicidad - 10); salud = max(0, salud - 5); }
      if (hambre >= 80 || felicidad <= 15 || energia <= 10) { salud = max(0, salud - 8); sonarTriste(); }
    }
  }

  // --- LECTURA DE BOTONES EN MODO NORMAL ---
  if (digitalRead(BTN_A) == LOW) { opcionMenu = (opcionMenu + 1) % TOTAL_OPCIONES; sonarPitido(800, 30); delay(200); }
  
  if (digitalRead(BTN_B) == LOW) {
    if (salud > 0) {
      if (opcionMenu == 0 && !estaDormido) { hambre = max(0, hambre - 25); sonarFeliz(); guardarPartida(); } 
      else if (opcionMenu == 1 && !estaDormido) { // JUGAR
        if (hambre < 80 && energia > 20) { energia -= 20; modoJuegoActivo = 1; rondaActual = 0; puntosJugador = 0; sonarPitido(1000, 50); } 
        else { sonarTriste(); }
      } 
      else if (opcionMenu == 2 && !estaDormido) { // CASINO TRAGAMONEDAS
        if (monedas >= 2) { monedas -= 2; ejecutarCasino(); } 
        else { sonarTriste(); }
      }
      else if (opcionMenu == 3) { salud = min(100, salud + 25); sonarFeliz(); guardarPartida(); } // CURAR
      else if (opcionMenu == 4) { // ACCIÓN ESPECIAL / TIENDA
        if (tienePopo) { tienePopo = false; felicidad = min(100, felicidad + 20); sonarFeliz(); }
        else if (estaDormido) { luzPrendida = !luzPrendida; sonarPitido(600, 100); }
        else {
          if (monedas >= 10) { monedas -= 10; felicidad = 100; hambre = min(100, hambre + 15); sonarFeliz(); } 
          else { sonarTriste(); }
        }
        guardarPartida();
      }
    }
    delay(200);
  }

  if (digitalRead(BTN_C) == LOW) {
    unsigned long tPulsado = millis();
    while(digitalRead(BTN_C) == LOW) {
      if(millis() - tPulsado > 2000) { // Si dejas presionado el Botón C por 2 segundos, abres el menú de Reset
        modoJuegoActivo = 3;
        sonarTriste();
        return;
      }
    }
    mostrandoStatus = !mostrandoStatus;
    sonarPitido(1000, 40);
  }

  // --- PANTALLA NOCTURNA ---
  if (!luzPrendida && estaDormido) {
    display.clearDisplay();
    display.fillRect(0, 0, 128, 64, SSD1306_BLACK);
    display.setCursor(108, 4); display.print(F("C")); display.setCursor(44, 25); display.print(F("Zzz..."));
    display.drawFastHLine(0, 49, 128, SSD1306_WHITE);
    display.setCursor(12, 54);
    if(opcionMenu == 4) display.print(F("< PRENDER LUZ >"));
    else display.print(F("<  DURMIENDO  >"));
    display.display();
    return;
  }

  if (salud <= 0) {
    display.clearDisplay(); display.setTextSize(2); display.setCursor(12, 15); display.println(F("GAME OVER"));
    display.setTextSize(1); display.setCursor(14, 45); display.print(F("Manten C: Reset"));
    display.display();
    return;
  }

  // --- MONITOREO DE ESTADÍSTICAS AMPLIADO ---
  if (mostrandoStatus) {
    display.clearDisplay(); display.setCursor(0, 0); display.println(F("---- ESTADISTICAS ----"));
    display.print(F("Edad:    ")); display.print(diasVividos); display.println(F(" dias"));
    display.print(F("Monedas: ")); display.print(monedas); display.println(F(" G"));
    display.print(F("Energia: ")); display.print(energia); display.println(F(" %"));
    display.print(F("Salud:   ")); display.print(salud); display.println(F(" %"));
    display.setCursor(0, 54); display.println(F("[C] Volver al juego"));
    display.display();
    return;
  }

  // --- RENDER DE INTERFAZ PRINCIPAL ---
  display.clearDisplay();
  display.setCursor(2, 0); display.print(F("H:")); display.print(hambre);
  display.setCursor(44, 0); display.print(F("F:")); display.print(felicidad);
  display.setCursor(86, 0); display.print(F("$:")); display.print(monedas);
  display.drawFastHLine(0, 10, 128, SSD1306_WHITE);

  if (tiempoActual - ultimaAnimacion > 400) { ultimaAnimacion = tiempoActual; frameAnimacion = (frameAnimacion + 1) % 2; }
  int offsetY = (frameAnimacion == 1 && !estaDormido) ? 2 : 0;

  // --- DIBUJO DE EVOLUCIONES MÚLTIPLES ---
  if (tipoEvolucion == 0) { // BEBÉ
    display.drawCircle(64, 30 - offsetY, 10, SSD1306_WHITE);
    if(!estaDormido){ display.fillRect(60, 27-offsetY, 2, 3, SSD1306_WHITE); display.fillRect(66, 27-offsetY, 2, 3, SSD1306_WHITE); display.drawFastHLine(62, 34-offsetY, 4, SSD1306_WHITE); }
    else { display.setCursor(78, 14); display.print(F("zZ")); }
  } 
  else if (tipoEvolucion == 1) { // NORMAL (Adulto Cuadrado)
    display.drawRoundRect(48, 20 - offsetY, 32, 24, 4, SSD1306_WHITE);
    if(!estaDormido){ display.fillRect(54, 26-offsetY, 3, 4, SSD1306_WHITE); display.fillRect(71, 26-offsetY, 3, 4, SSD1306_WHITE); display.drawFastHLine(59, 35-offsetY, 10, SSD1306_WHITE); }
    else { display.setCursor(84, 12); display.print(F("Zz")); }
  } 
  else if (tipoEvolucion == 2) { // EVOLUCIÓN: ÁNGEL (Alas y Aureola)
    display.drawCircle(64, 32 - offsetY, 11, SSD1306_WHITE); // Cuerpo
    display.drawEllipse(64, 16 - offsetY, 8, 2, SSD1306_WHITE); // Aureola brillante
    display.drawFastVLine(49, 26 - offsetY, 8, SSD1306_WHITE); // Ala Izquierda
    display.drawFastVLine(79, 26 - offsetY, 8, SSD1306_WHITE); // Ala Derecha
    if(!estaDormido){ display.fillRect(59, 29-offsetY, 2, 3, SSD1306_WHITE); display.fillRect(67, 29-offsetY, 2, 3, SSD1306_WHITE); display.drawPixel(64, 36-offsetY, SSD1306_WHITE); }
  } 
  else if (tipoEvolucion == 3) { // EVOLUCIÓN: MONSTRUO (Picos y cuernos)
    display.drawRect(48, 22 - offsetY, 32, 22, SSD1306_WHITE);
    display.drawTriangle(48, 22-offsetY, 44, 14-offsetY, 52, 22-offsetY, SSD1306_WHITE); // Cuerno I
    display.drawTriangle(80, 22-offsetY, 84, 14-offsetY, 76, 22-offsetY, SSD1306_WHITE); // Cuerno D
    if(!estaDormido){ 
      display.fillRect(53, 27-offsetY, 4, 2, SSD1306_WHITE); display.fillRect(71, 27-offsetY, 4, 2, SSD1306_WHITE); // Ojos enojados
      display.drawFastHLine(56, 36-offsetY, 16, SSD1306_WHITE); 
    }
  }

  if (tienePopo) { display.setCursor(14, 32); display.print(F("P_P")); }

  // --- MENÚ CON SCROLL HORIZONTAL COMPACTO ---
  display.drawFastHLine(0, 49, 128, SSD1306_WHITE);
  display.setCursor(6, 54); display.print(F("< "));
  
  if (opcionMenu == 4) {
    if (tienePopo)        display.print(F("LIMPIAR POPO"));
    else if (estaDormido) display.print(F("APAGAR LUZ"));
    else                  display.print(F("COMPRA DULCE$10")); 
  } else {
    display.print(nombresMenu[opcionMenu]);
  }
  display.setCursor(114, 54); display.print(F(" >"));

  display.display();
}