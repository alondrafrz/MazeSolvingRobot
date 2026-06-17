  /*
   Seguidor de línea con PID
   Sigue línea blanca

   Felipe Ramírez Alondra Navila
*/

#include <WiFi.h>
#include <WifiUdp.h>

const char* ssid = "Incubot";
const char* password = "@El0t1t0";

const char* ipCompu = "192.168.3.18";
const int puertoUdp = 1234;

WiFiUDP udp;

// Pines sensores
const int pinSensores[3] = {0, 1, 2}; // IZQ, CEN, DER

// Pines motores
const int IN1 = 16;
const int IN2 = 23;
const int IN3 = 22;
const int IN4 = 21;

// Velocidad base
int velocidadBase = 120;
float compensacionDer = 0.85;

// Variables PID
float Kp = 6.5 ;
float Ki = 0.058;
float Kd = 0.12 ; //0.1;

#define numDelay 3.5

float error = 0;
float errorAnterior = 0; // Guarda el error del ciclo anterior para calcular la tasa de c  ambio
float integral = 0;
float derivada = 0;
float PID = 0;

// control de tiempo
unsigned long tiempoAntMicros = 0;

// Última dirección conocida
int ultimoError = 0;

// Variables calibración
int minSensores[3] = {4095, 4095, 4095};
int maxSensores[3] = {0, 0, 0};

long sumaLecturas[3] = {0, 0, 0};
long contadorMuestras = 0;

int umbrales[3] = {130, 130, 130};


void setup() {

  Serial.begin(115200);

  // Sensores
  for(int i = 0; i < 3; i++) {
    pinMode(pinSensores[i], INPUT);
  }

  // Motores
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  detener();

// Iniciamos conexión WiFi
  Serial.print("Conectando a ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while(WiFi.status()!= WL_CONNECTED) {
     delay(500);
     Serial.print(".");
  }

  Serial.println("");
  Serial.println("Conectado");

  
  delay(3000);
  ejecutarCalibracion();
  delay(1000);

  // inicializamos el cronómetro
  tiempoAntMicros = micros();
  
}


void loop() {
  int val[3];
  // Leer sensores
  for(int i = 0; i < 3; i++) {
    val[i] = analogRead(pinSensores[i]);
  }

  // Detectar línea blanca
  bool izq = val[0] <= umbrales[0];
  bool cen = val[1] <= umbrales[1];
  bool der = val[2] <= umbrales[2];

  // Calcular error
  if (!izq && cen && !der) { error = 0; }       // centrado perfecto
  else if (izq && cen && !der) { error = -0.5; } // ligeramente a la izquierda
  else if (!izq && cen && der) { error = 0.5; }  // ligeramente a la derecha
  else if (izq && !cen && !der) { error = -1; }  // muy a la izquierda
  else if (!izq && !cen && der) { error = 1; }   // muy a la derecha
  else if (izq && cen && der) { error = 0; }     // toda la línea debajo
  else {

    // Buscar línea girando sobre su propio eje según la última dirección
    if(ultimoError < 0) {
      // La línea estaba a la izquierda, gira hacia la izquierda
      // Motor izquierdo hacia atrás, derecho hacia adelante
      moverMotores(-85, 85); 
    }
    else {
      // La línea estaba a la derecha, gira hacia la derecha
      // Motor izquierdo hacia adelante, derecho hacia atrás
      moverMotores(85, -85); 
    }
    tiempoAntMicros = micros(); // si perdemos la linea, reinicia el tiempo para que al volver 
    // dt no sea muy grande
    return; // Evita que se calcule el PID si no hay línea
  }

  
  // Guardar último error
  ultimoError = error;

  unsigned long tiempoActualMicros = micros();
  float dt = (float)(tiempoActualMicros - tiempoAntMicros)/1000000.0; // calculamos tiempo trasncurrido en seg
  tiempoAntMicros = tiempoActualMicros;

  /*if(dt <= 0){
    dt = 0.005;
  }*/

  // PID

  //Proporcional:
  float proporcional = Kp * error;
  //integral:
  integral += error * dt;
  integral = constrain(integral, -100.0, 100.0); //restringe el crecimiento de la integral
  //derivada:
  derivada = (error - errorAnterior) / dt;

  PID = proporcional + (Ki * integral) + (Kd * derivada);
  errorAnterior = error;


  // Ajustar motores
  int velIzq = velocidadBase + PID;
  int velDer = velocidadBase - PID;

  // Rango ampliado para aprovechar motores en reversa durante curvas
  velIzq = constrain(velIzq, -255, 255);
  velDer = constrain(velDer, -255, 255);
  moverMotores(velIzq, velDer);


  //Empaquetamiento por UDP
  udp.beginPacket(ipCompu, puertoUdp);

  udp.print(val[0]);
  udp.print(",");
  udp.print(val[1]);
  udp.print(",");
  udp.print(val[2]);
  udp.print(",");
  udp.print(velIzq);
  udp.print(",");
  udp.println(velDer);

  udp.endPacket();
  
  
  delay(numDelay);
}


// Calibración
void ejecutarCalibracion() {
  for (int i = 0; i < 3; i++) {
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
    // Umbral automático
    umbrales[i] = (minSensores[i] + maxSensores[i]) / 2;
  }
}

// Captura lecturas calibración
void capturarProcesar(int duracionMs) {
  unsigned long inicio = millis();
  while (millis() - inicio < duracionMs) {
    for (int i = 0; i < 3; i++) {
      int lectura = analogRead(pinSensores[i]);

      // Guardar mínimos
      if (lectura < minSensores[i]) {
        minSensores[i] = lectura;
      }

      // Guardar máximos
      if (lectura > maxSensores[i]) {
        maxSensores[i] = lectura;
      }
      sumaLecturas[i] += lectura;
    }
    contadorMuestras++;
    delay(5);
  }
}


// Movimiento PID 
void moverMotores(int velIzq, int velDer) {
  velDer = (int)(velDer * compensacionDer); //frenar el motor derecho

  // Control del Motor Izquierdo
  if (velIzq >= 0) {
    analogWrite(IN1, velIzq);
    analogWrite(IN2, 0);
  } else {
    // Velocidad negativa: retrocede
    analogWrite(IN1, 0);
    analogWrite(IN2, -velIzq); 
  }

  // Control del Motor Derecho
  if (velDer >= 0) {
    analogWrite(IN3, 0);
    analogWrite(IN4, velDer);
  } else {
    // Velocidad negativa: retrocede
    analogWrite(IN3, -velDer);
    analogWrite(IN4, 0);
  }
}

// Funciones básicas
void avanzar() {
  moverMotores(velocidadBase, velocidadBase);
}
void atras() {
  moverMotores(-velocidadBase, -velocidadBase);
}
void detener() {
  moverMotores(0, 0);
}
