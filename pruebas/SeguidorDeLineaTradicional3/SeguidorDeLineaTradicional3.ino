/*
   Seguidor de línea con PID
   ESP32-C6

   Felipe Ramírez Alondra Navila
   20 de abril de 2026
   Versión 4 - Implementación de control PID
*/

// ======================
// Pines sensores
// ======================

const int pinSensores[3] = {0, 1, 2}; // IZQ, CEN, DER

// ======================
// Pines motores
// ======================

const int IN1 = 16;
const int IN2 = 23;
const int IN3 = 22;
const int IN4 = 21;

// ======================
// PWM ESP32-C6
// ======================

const int freq = 5000;
const int resolution = 8;

// Canales PWM
const int chIN1 = 0;
const int chIN2 = 1;
const int chIN3 = 2;
const int chIN4 = 3;

// ======================
// Velocidad base
// ======================

int velocidadBase = 85;

// ======================
// Variables PID
// ======================

float Kp = 35;
float Ki = 0;
float Kd = 15;

float error = 0;
float errorAnterior = 0;
float integral = 0;
float derivada = 0;
float PID = 0;

// ======================
// Calibración sensores
// ======================

int minSensores[3] = {4095, 4095, 4095};
int maxSensores[3] = {0, 0, 0};

int umbrales[3] = {200, 200, 200};

// ======================
// SETUP
// ======================

void setup() {

  Serial.begin(115200);

  // Sensores
  for (int i = 0; i < 3; i++) {
    pinMode(pinSensores[i], INPUT);
  }

  // PWM motores
  ledcAttach(IN1, freq, resolution);
  ledcAttach(IN2, freq, resolution);
  ledcAttach(IN3, freq, resolution);
  ledcAttach(IN4, freq, resolution);

  delay(3000);

  ejecutarCalibracion();

  delay(1000);
}

// ======================
// LOOP PRINCIPAL
// ======================

void loop() {

  int val[3];

  // Lectura sensores
  for (int i = 0; i < 3; i++) {
    val[i] = leerSensor(pinSensores[i]);
  }

  // Mostrar lecturas (debug)
  Serial.print(val[0]);
  Serial.print("  ");
  Serial.print(val[1]);
  Serial.print("  ");
  Serial.println(val[2]);

  // Calcular error
  error = calcularError(val);

  // PID
  integral += error;

  derivada = error - errorAnterior;

  PID = (Kp * error) +
        (Ki * integral) +
        (Kd * derivada);

  errorAnterior = error;

  // Ajustar velocidades
  int velIzq = velocidadBase - PID;
  int velDer = velocidadBase + PID;

  // Limitar velocidades
  velIzq = constrain(velIzq, 0, 255);
  velDer = constrain(velDer, 0, 255);

  // Mover motores
  moverMotores(velIzq, velDer);
}

// ======================
// FILTRO SENSORES
// ======================

int leerSensor(int pin) {

  long suma = 0;

  for (int i = 0; i < 5; i++) {
    suma += analogRead(pin);
  }

  return suma / 5;
}

// ======================
// CÁLCULO ERROR
// ======================

float calcularError(int val[]) {

  bool izq = val[0] <= umbrales[0];
  bool cen = val[1] <= umbrales[1];
  bool der = val[2] <= umbrales[2];

  // Centro perfecto
  if (cen && !izq && !der) {
    return 0;
  }

  // Izquierda
  if (izq && !cen) {
    return -1;
  }

  // Derecha
  if (der && !cen) {
    return 1;
  }

  // Entre izquierda y centro
  if (izq && cen) {
    return -0.5;
  }

  // Entre derecha y centro
  if (der && cen) {
    return 0.5;
  }

  // No detecta línea
  return errorAnterior;
}

// ======================
// CONTROL MOTORES
// ======================

void moverMotores(int velIzq, int velDer) {

  // Motor izquierdo adelante
  ledcWrite(chIN1, velIzq);
  ledcWrite(chIN2, 0);

  // Motor derecho adelante
  ledcWrite(chIN3, 0);
  ledcWrite(chIN4, velDer);
}

// ======================
// DETENER
// ======================

void detenerMotores() {

  ledcWrite(chIN1, 0);
  ledcWrite(chIN2, 0);
  ledcWrite(chIN3, 0);
  ledcWrite(chIN4, 0);
}

// ======================
// CALIBRACIÓN
// ======================

void ejecutarCalibracion() {

  Serial.println("=== CALIBRANDO ===");

  unsigned long inicio = millis();

  while (millis() - inicio < 5000) {

    // Movimiento suave durante calibración
    moverMotores(70, 40);

    for (int i = 0; i < 3; i++) {

      int lectura = analogRead(pinSensores[i]);

      if (lectura < minSensores[i]) {
        minSensores[i] = lectura;
      }

      if (lectura > maxSensores[i]) {
        maxSensores[i] = lectura;
      }
    }

    delay(5);
  }

  detenerMotores();

  // Calcular umbrales
  for (int i = 0; i < 3; i++) {

    umbrales[i] =
      (minSensores[i] + maxSensores[i]) / 2;
  }

  // Mostrar resultados
  const char* etiquetas[] = {"IZQ", "CEN", "DER"};

  for (int i = 0; i < 3; i++) {

    Serial.print(etiquetas[i]);

    Serial.print(" | Min: ");
    Serial.print(minSensores[i]);

    Serial.print(" | Max: ");
    Serial.print(maxSensores[i]);

    Serial.print(" | Umbral: ");
    Serial.println(umbrales[i]);
  }

  Serial.println("=== FIN CALIBRACION ===");
}
