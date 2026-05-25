/*
 * Programa que se encarga de encontrar los umbrales
 * de los sensores TCRT5000
 * 
 * Felipe Ramírez Alondra Navila
 * 12 de marzo de 2026
 * Versión definitiva
 */

// Definimos los pines de los sensores para el ESP32-C6
const int pinUFL = 0; // Izquierda
const int pinUFC = 1; // Centro
const int pinUFR = 2; // Derecha

void setup() {
  // En el ESP32-C6, el Serial suele ir a 115200
  Serial.begin(115200); 
  
  // Configuramos como entrada
  pinMode(pinUFL, INPUT);
  pinMode(pinUFC, INPUT);
  pinMode(pinUFR, INPUT);
  
  Serial.println("--- Iniciando lectura de sensores ---");
}

void loop() {
  // Leemos los valores analógicos (0 - 4095)
  int valUFL = analogRead(pinUFL);
  int valUFC = analogRead(pinUFC);
  int valUFR = analogRead(pinUFR);

  // Mostramos los resultados en una sola línea para comparar mejor
  Serial.print("IZQ (UFL): "); Serial.print(valUFL);
  Serial.print("\t CEN (UFC): "); Serial.print(valUFC);
  Serial.print("\t DER (UFR): "); Serial.println(valUFR);

  delay(300); // Un poco de tiempo para poder leer
}
