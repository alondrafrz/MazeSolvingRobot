 /*
   Programa de un seguidor de línea tardicional
   
   Felipe Ramírez Alondra Navila
   20 de abril de 2026
   Versión 2. Seguidor de linea tradicional con subrutina de 
   calibración automática.
*/

// Definimos  los pines de los sensores y motores para el ESP32-C6
const int pinSensores[3] = {0, 1, 2}; // IZQ, CEN, DER
const int IN1 = 16, IN2 = 23, IN3 = 22, IN4 = 21;

int motorIzq = 95;  
int motorDer = 80;

// Variables de calibración independiente
int minSensores[3] = {4095, 4095, 4095};
int maxSensores[3] = {0, 0, 0};
long sumaLecturas[3] = {0, 0, 0};
long contadorMuestras = 0;
int umbrales[3] = {215, 215, 215}; // Valores iniciales

void setup() {
  Serial.begin(115200);
  
  for(int i=0; i<3; i++) {
    pinMode(pinSensores[i], INPUT);
  }
  
  pinMode(IN1, OUTPUT); 
  pinMode(IN2, OUTPUT); 
  pinMode(IN3, OUTPUT); 
  pinMode(IN4, OUTPUT);

  
  delay(3000);
  ejecutarCalibracion();
  delay(1000);
}

void loop() {
  // Leemos los 3 sensores
  int val[3];
  for(int i=0; i<3; i++) {
    val[i] = analogRead(pinSensores[i]);
  }

  /* Lógica usando sus respectivos umbrales
  if (val[1] > umbrales[11 && val[1] > umbrales[1] && val[2] > umbrales[2]) {
    detener();
  }
  else if (val[0] <= umbrales[0] && val[1] > umbrales[1] && val[2] > umbrales[2]) {
    izquierda();
  }
  else if (val[0] > umbrales[0] && val[1] <= umbrales[1] && val[2] > umbrales[2]) {
    avanzar();
  }
  else if (val[0] > umbrales[0] && val[1] > umbrales[1] && val[2] <= umbrales[2]) {
    derecha();
  }
  else {
    detener();
  }
  
  delay(20);
  */

  if (val[1] <= umbrales[1]) {
    avanzar(); // Línea blanca en el centro
  } 
  else if (val[0] <= umbrales[0]) {
    delay(12);
    izquierda(); // Línea blanca a la izquierda
  }
  else if (val[2] <= umbrales[2]) {
    delay(12);
    derecha(); // Línea blanca a la derecha
  }
  else {
    detener(); // No ve línea
  }

  delay(15);
}

// Subrutina de calibración
void ejecutarCalibracion() {
  
  for (int i = 0; i < 5; i++) {
    avanzar(); 
    delay(50);
    capturarProcesar(400); 
    detener(); 
    delay(200);
    
    atras(); 
    delay(50);  
    capturarProcesar(400); 
    detener(); 
    delay(200);
  }

  const char* etiquetas[] = {"IZQ", "CEN", "DER"};
  
  for (int i = 0; i < 3; i++) {
    if(contadorMuestras > 0) {
      umbrales[i] = (int)(sumaLecturas[i] / contadorMuestras);
    }
    Serial.print(etiquetas[i]);
    Serial.print(" | Min: "); 
    Serial.print(minSensores[i]);
    Serial.print(" | Max: "); 
    Serial.print(maxSensores[i]);
    Serial.print(" | Nuevo Umbral (Prom): "); 
    Serial.println(umbrales[i]);
  }
  
}

void capturarProcesar(int duracionMs) {
  unsigned long inicio = millis();
  while (millis() - inicio < duracionMs) {
    for (int i = 0; i < 3; i++) {
      int lectura = analogRead(pinSensores[i]);
      
      if (lectura < minSensores[i]) minSensores[i] = lectura;
      if (lectura > maxSensores[i]) maxSensores[i] = lectura;

      sumaLecturas[i] += lectura;
    }
    contadorMuestras++;
    delay(5);
  }
}

// --- FUNCIONES DE MOVIMIENTO ---

void avanzar() {
  analogWrite(IN1, motorIzq); 
  analogWrite(IN2, 0);
  analogWrite(IN3, 0); 
  analogWrite(IN4, motorDer);
}

void atras() {
  analogWrite(IN1, 0); 
  analogWrite(IN2, motorIzq);
  analogWrite(IN3, motorDer); 
  analogWrite(IN4, 0);
}

void izquierda() {
  analogWrite(IN1, 0); 
  analogWrite(IN2, motorIzq);
  analogWrite(IN3, 0); 
  analogWrite(IN4, motorDer);
}

void derecha() {
  analogWrite(IN1, motorIzq); 
  analogWrite(IN2, 0);
  analogWrite(IN3, motorDer);
  analogWrite(IN4, 0);
}

void detener() {
  analogWrite(IN1, 0); 
  analogWrite(IN2, 0);
  analogWrite(IN3, 0); 
  analogWrite(IN4, 0);
}
