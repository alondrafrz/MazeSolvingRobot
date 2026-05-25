 /*
   Programa de un seguidor de línea tardicional

   Felipe Ramírez Alondra Navila
   19 de marzo de 2026
   Versión integrada
*/
  
// Definimos  los pines de los sensores y motores para el ESP32-C6
/*
const int pinUFL = 0; //Sensor izquierdo
const int pinUFC = 1; // Sensor central
const int pinUFR = 2; // Sensor derecho
*/
const int pinSensores[3] = {0, 1, 2}

const int IN1 = 16;
const int IN2 = 23;
const int IN3 = 22;
const int IN4 = 21;

int motorIzq = 95 ; // PWM del motor izquierdo
int motorDer = 80; // PWM del motor derecho
//int umbral = 215;

// Calibración a partir de la subrutina
int minSensores[3] = {4095, 4095, 4095};
int maxSensores[3] = {0, 0, 0};
int umbrales[3] = {215, 215, 215}; // Inicializamos umbrales

void setup() {
  // Inicializamos el serial
  Serial.begin(115200);

  // Configuramos los pines de los sensores como entradas
  for (int i = 0; i < 3; i++) {
    pinMode(pinSensores[i], INPUT);
  }
  

  // Configuramos los pines de los motores como salidas
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // Realiza calibración al iniciar
  Serial.println("Coloca el robot en la pista...");
  delay(3000);
  ejecutarCalibracion();

}

void avanzar() {
  // motor A (izquierdo)
  analogWrite(IN1, motorIzq);
  analogWrite(IN2, 0);
  // motor B (derecho)
  analogWrite(IN3, 0);
  analogWrite(IN4, motorDer);
}

void atras() {
  // motor A (izquierdo)
  analogWrite(IN1, 0);
  analogWrite(IN2, motorIzq);
  // motor B (derecho)
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

void loop() {
  /*
   //Leemos los valores de los sensores
    int valUFL = analogRead(pinUFL);
    int valUFC = analogRead(pinUFC);
    int valUFR = analogRead(pinUFR);

    Serial.print("IZQ: "); 
    Serial.print(valUFL);
    Serial.print(" | CEN: ");
    Serial.print(valUFC);
    Serial.print(" | DER: ");
    Serial.println(valUFR);

    if (valUFL > umbral && valUFC > umbral && valUFR > umbral) {
    detener();
  }

  else if (valUFL <= umbral && valUFC > umbral && valUFR > umbral) {
    delay(11);
    izquierda();
    
  }

  else if (valUFL > umbral && valUFC <= umbral && valUFR > umbral) {
    avanzar();
  }

  else if (valUFL > umbral && valUFC > umbral && valUFR <= umbral) {
    delay(11);
    derecha();
    
  }

  else if (valUFL <= umbral && valUFC <= umbral && valUFR <= umbral) {
    detener();
  }

  else {
    Serial.println("no");
  }

  delay(25);

  avanzar();
  delay(25 );
  
/*
  if (analogRead(pinUFL) > umbral && analogRead(pinUFC) > umbral && analogRead(pinUFR) > umbral) {
    detener();
  }

  else if (analogRead(pinUFL) <= umbral && analogRead(pinUFC) > umbral && analogRead(pinUFR) > umbral) {
    izquierda();
  }

  else if (analogRead(pinUFL) > umbral && analogRead(pinUFC) <= umbral && analogRead(pinUFR) > umbral) {
    avanzar();
  }

  else if (analogRead(pinUFL) > umbral && analogRead(pinUFC) > umbral && analogRead(pinUFR) <= umbral) {
    derecha();
  }

  else if (analogRead(pinUFL) <= umbral && analogRead(pinUFC) <= umbral && analogRead(pinUFR) <= umbral) {
    detener();
  }

  else {
    Serial.println("no");
  }
  */
}

// Subrutina
void ejecutarCalibracion() {
  Serial.println("Calibrando: 5 ciclos adelante/atrás...");
  
  for (int i = 0; i < 5; i++) {
    avanzar();
    capturarDatos(400); 
    detener();
    delay(200);

    atras();
    capturarDatos(400);
    detener();
    delay(200);
  }

  if (contadorMuestras > 0) {
    umbral = (calibMin + calibMax) / 2;
    float promedioFinal = (float)sumaLecturas / contadorMuestras; // Cambié el nombre para no chocar con sumaLecturas
    
    Serial.println("--- RESULTADOS CALIBRACIÓN ---");
    Serial.print("Mínimo (Blanco): "); Serial.println(calibMin);
    Serial.print("Máximo (Negro): "); Serial.println(calibMax);
    Serial.print("Promedio General: "); Serial.println(promedioFinal);
    Serial.print("Nuevo Umbral Sugerido: "); Serial.println(umbral);
    Serial.println("------------------------------");
  }
}

void capturarDatos(int duracionMs) {
  unsigned long tiempoInicio = millis();
  while (millis() - tiempoInicio < duracionMs) {
    int v[3] = {analogRead(pinUFL), analogRead(pinUFC), analogRead(pinUFR)};
    for (int i = 0; i < 3; i++) {
      if (v[i] < calibMin) calibMin = v[i];
      if (v[i] > calibMax) calibMax = v[i];
      sumaLecturas += v[i];
      contadorMuestras++;
    }
    delay(5);
  }
}
