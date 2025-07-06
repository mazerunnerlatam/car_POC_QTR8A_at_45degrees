/*
  ==================================================================================
  Código de Control para Robot Seguidor de Línea - VERSIÓN QTR-8A DEFINITIVA
  Plataforma: Arduino
  Versión: 4.3 (Lógica de Giro Corregida)

  Descripción:
  Esta versión corrige la lógica en las funciones de giro fuerte para que ambas
  ruedas avancen a diferentes velocidades, en lugar de pivotar. También se asegura
  de usar la constante VELOCIDAD_GIRO_FUERTE.
  ==================================================================================
*/

#include <QTRSensors.h>

// ===== SECCIÓN 1: PARÁMETROS DE AJUSTE Y HARDWARE =====

// --- Configuración del Sensor QTR-8A ---
#define SENSOR_COUNT 8 
#define PIN_EMITTER 7 // Pin que controla los emisores IR del sensor
QTRSensors qtr;
uint16_t sensorValues[SENSOR_COUNT];
const int UMBRAL_NEGRO = 900;

// --- Velocidades de Maniobra ---
const int VELOCIDAD_BASE = 96;
const int VELOCIDAD_GIRO = 92;
const int VELOCIDAD_GIRO_FUERTE = VELOCIDAD_GIRO + 16; // Resultado es 108
const int VELOCIDAD_BUSQUEDA = 90;
const int VELOCIDAD_MINIMA = 80;

// --- Tiempos de Maniobra (en milisegundos) ---
const int TIEMPO_MANIOBRA = 200;
const int PAUSA_ENTRE_ACCIONES = 300; // SUGERENCIA: Un valor más bajo para mayor fluidez
const int TIEMPO_BUSQUEDA = 150;

// --- Pines de Motores ---
#define MOTOR_IZQ_IN1 24
#define MOTOR_IZQ_IN2 25
#define MOTOR_IZQ_ENA 2
#define MOTOR_DER_IN3 26
#define MOTOR_DER_IN4 27
#define MOTOR_DER_ENB 3
const int STATUS_LED_PIN = 13;

// --- Variables Globales ---
int lineaPerdidaContador = 0;
#define MAXlineaPerdidaContador 7


// ===== SECCIÓN 2: FUNCIONES DE ACCIÓN DISCRETA (MOVIMIENTO) =====

void accion_Detener() {
  digitalWrite(MOTOR_IZQ_IN1, LOW);
  digitalWrite(MOTOR_IZQ_IN2, LOW);
  analogWrite(MOTOR_IZQ_ENA, 0);
  digitalWrite(MOTOR_DER_IN3, LOW);
  digitalWrite(MOTOR_DER_IN4, LOW);
  analogWrite(MOTOR_DER_ENB, 0);
}

void accion_AvanzarRecto() {
  Serial.println("-> Accion: Avanzar Recto");
  digitalWrite(MOTOR_IZQ_IN1, HIGH);
  digitalWrite(MOTOR_IZQ_IN2, LOW);
  digitalWrite(MOTOR_DER_IN3, HIGH);
  digitalWrite(MOTOR_DER_IN4, LOW);
  analogWrite(MOTOR_IZQ_ENA, VELOCIDAD_BASE);
  analogWrite(MOTOR_DER_ENB, VELOCIDAD_BASE);
  delay(TIEMPO_MANIOBRA);
}

void accion_GiroSuaveDerecha() {
  Serial.println("-> Accion: Giro Suave Derecha");
  digitalWrite(MOTOR_IZQ_IN1, HIGH);
  digitalWrite(MOTOR_IZQ_IN2, LOW);
  digitalWrite(MOTOR_DER_IN3, HIGH);
  digitalWrite(MOTOR_DER_IN4, LOW);
  analogWrite(MOTOR_IZQ_ENA, VELOCIDAD_GIRO);
  analogWrite(MOTOR_DER_ENB, VELOCIDAD_MINIMA);
  delay(TIEMPO_MANIOBRA);
}

void accion_GiroSuaveIzquierda() {
  Serial.println("-> Accion: Giro Suave Izquierda");
  digitalWrite(MOTOR_IZQ_IN1, HIGH);
  digitalWrite(MOTOR_IZQ_IN2, LOW);
  digitalWrite(MOTOR_DER_IN3, HIGH);
  digitalWrite(MOTOR_DER_IN4, LOW);
  analogWrite(MOTOR_IZQ_ENA, VELOCIDAD_MINIMA);
  analogWrite(MOTOR_DER_ENB, VELOCIDAD_GIRO);
  delay(TIEMPO_MANIOBRA);
}

// ========= INICIO DE SECCIÓN CORREGIDA =========
void accion_GiroFuerteDerecha() {
  Serial.println("-> Accion: GIRO FUERTE DERECHA");
  // CORRECCIÓN 1: Ambas ruedas hacia adelante
  digitalWrite(MOTOR_IZQ_IN1, HIGH);
  digitalWrite(MOTOR_IZQ_IN2, LOW);
  digitalWrite(MOTOR_DER_IN3, HIGH); // <-- Rueda derecha ahora hacia ADELANTE
  digitalWrite(MOTOR_DER_IN4, LOW);

  // CORRECCIÓN 2: Usar las velocidades correctas
  analogWrite(MOTOR_IZQ_ENA, VELOCIDAD_GIRO_FUERTE); // <-- Rueda externa más rápida
  analogWrite(MOTOR_DER_ENB, VELOCIDAD_MINIMA);      // <-- Rueda interna a velocidad mínima
  delay(TIEMPO_MANIOBRA);
}

void accion_GiroFuerteIzquierda() {
  Serial.println("-> Accion: GIRO FUERTE IZQUIERDA");
  // CORRECCIÓN 1: Ambas ruedas hacia adelante
  digitalWrite(MOTOR_IZQ_IN1, HIGH); // <-- Rueda izquierda ahora hacia ADELANTE
  digitalWrite(MOTOR_IZQ_IN2, LOW);
  digitalWrite(MOTOR_DER_IN3, HIGH);
  digitalWrite(MOTOR_DER_IN4, LOW);
  
  // CORRECCIÓN 2: Usar las velocidades correctas
  analogWrite(MOTOR_IZQ_ENA, VELOCIDAD_MINIMA);      // <-- Rueda interna a velocidad mínima
  analogWrite(MOTOR_DER_ENB, VELOCIDAD_GIRO_FUERTE); // <-- Rueda externa más rápida
  delay(TIEMPO_MANIOBRA);
}
// ========= FIN DE SECCIÓN CORREGIDA =========

void accion_BuscarAdelante() {
  Serial.print("-> Accion: Buscando linea... (Intento ");
  Serial.print(lineaPerdidaContador);
  Serial.println(")");
  digitalWrite(MOTOR_IZQ_IN1, HIGH);
  digitalWrite(MOTOR_IZQ_IN2, LOW);
  digitalWrite(MOTOR_DER_IN3, HIGH);
  digitalWrite(MOTOR_DER_IN4, LOW);
  analogWrite(MOTOR_IZQ_ENA, VELOCIDAD_BUSQUEDA);
  analogWrite(MOTOR_DER_ENB, VELOCIDAD_BUSQUEDA);
  delay(TIEMPO_BUSQUEDA);
}


// ===== SECCIÓN 3: SETUP Y LOOP PRINCIPAL =====

void startupLEDPattern() {
  for (int i = 0; i < 5; i++) {
    digitalWrite(STATUS_LED_PIN, HIGH); delay(250);
    digitalWrite(STATUS_LED_PIN, LOW); delay(250);
  }
  digitalWrite(STATUS_LED_PIN, HIGH); delay(1500);
  digitalWrite(STATUS_LED_PIN, LOW); delay(1500);
}

void setup() {
  Serial.begin(9600);
  
  pinMode(MOTOR_IZQ_IN1, OUTPUT);
  pinMode(MOTOR_IZQ_IN2, OUTPUT);
  pinMode(MOTOR_IZQ_ENA, OUTPUT);
  pinMode(MOTOR_DER_IN3, OUTPUT);
  pinMode(MOTOR_DER_IN4, OUTPUT);
  pinMode(MOTOR_DER_ENB, OUTPUT);
  pinMode(STATUS_LED_PIN, OUTPUT);
  
  digitalWrite(STATUS_LED_PIN, LOW); 
  delay(1500);
  
  qtr.setTypeAnalog();
  qtr.setSensorPins((const uint8_t[]){A0, A1, A2, A3, A4, A5, A6, A7}, SENSOR_COUNT);
  qtr.setEmitterPin(PIN_EMITTER);

  Serial.println("--- SISTEMA DE CONTROL INICIADO (V4.3 - Corregido) ---");
  Serial.println("Sensor: QTR-8A");
  Serial.println("Logica: Busqueda de linea y parada de emergencia");
  Serial.println("-------------------------------------------------");
  Serial.println("Recordatorio: >900 es NEGRO (linea), <700 es BLANCO.");
  Serial.println("VELOCIDAD MINIMA DE MOTOR: 80");
  Serial.println("-------------------------------------------------");
  
  Serial.println("Coloque el robot en la pista. Iniciando secuencia de LED...");
  startupLEDPattern();
  Serial.println("¡En marcha!");
}


void loop() {
  qtr.read(sensorValues);

  int s[SENSOR_COUNT];
  bool lineaDetectada = false;
  
  for(uint8_t i = 0; i < SENSOR_COUNT; i++) {
    if (sensorValues[i] > UMBRAL_NEGRO) {
      s[i] = 0;
      lineaDetectada = true;
    } else {
      s[i] = 1;
    }
  }

  Serial.print("Estado Binario: [");
  for(uint8_t i = 0; i < SENSOR_COUNT; i++) {
    Serial.print(s[i]);
  }
  Serial.print("] ");

  if (lineaDetectada) {
    lineaPerdidaContador = 0;

    if (s[0] == 1 && s[1] == 1 && s[2] == 1 && s[3] == 0 && s[4] == 0 && s[5] == 1 && s[6] == 1 && s[7] == 1) {
      accion_AvanzarRecto();
    }
    else if (s[0] == 1 && s[1] == 1 && s[2] == 0 && s[3] == 0 && s[4] == 1 && s[5] == 1 && s[6] == 1 && s[7] == 1) {
      accion_GiroSuaveIzquierda();
    }
    else if (s[0] == 1 && s[1] == 1 && s[2] == 1 && s[3] == 1 && s[4] == 0 && s[5] == 0 && s[6] == 1 && s[7] == 1) {
      accion_GiroSuaveDerecha();
    }
    else if (s[0] == 0 || s[1] == 0) {
      accion_GiroFuerteIzquierda();
    }
    else if (s[6] == 0 || s[7] == 0) {
      accion_GiroFuerteDerecha();
    }
    else {
      accion_AvanzarRecto();
    }
  } 
  else {
    lineaPerdidaContador++;
    
    if (lineaPerdidaContador >= MAXlineaPerdidaContador) {
      Serial.println("!!! PARADA DE EMERGENCIA: Linea no encontrada !!!");
      accion_Detener();
      while(true) {}
    } else {
      accion_BuscarAdelante();
    }
  }

  accion_Detener();
  delay(PAUSA_ENTRE_ACCIONES);
}