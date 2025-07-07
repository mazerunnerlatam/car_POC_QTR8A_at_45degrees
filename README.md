# car_POC_QTR8A_at_45degrees

Commit db44b2c  solo funciona bien cuando esta centrado en los casos extremos o bordes o curva no funciona
<br> Vamos a hacer un refactor con un proporcional
<br>
<br>
<br> Codigo NO TESTADO -  Migramos a 6 IRs con distancia D = d \
<br> es decir la distancia de la linea es aprox similar a la distancia de los sensores
<br>
<br>
/*
  ==================================================================================
  Código de Control para Robot Seguidor de Línea - VERSIÓN PROPORCIONAL
  Plataforma: Arduino
  Versión: 5.1 (Control Proporcional 'P' con Modo Debug Paso a Paso)

  Descripción:
  Añade un modo de depuración que permite ejecutar el loop ciclo a ciclo
  presionando una tecla en el Monitor Serie. Activar cambiando la constante
  MODO_DEBUG_PASO_A_PASO a 1.
  ==================================================================================
*/

#include <QTRSensors.h>

// ===== SECCIÓN 1: PARÁMETROS DE AJUSTE Y HARDWARE =====

// --- PARÁMETROS DE CONTROL PROPORCIONAL ---
#define MODO_DEBUG_PASO_A_PASO 1 // <<< INTERRUPTOR: 1 para Debug ON, 0 para Normal OFF

const float Kp = 0.05; // Constante de Proporcionalidad.
const int VELOCIDAD_MAXIMA = 120; // Velocidad base del robot en PWM (0-255)

// --- Configuración del Sensor QTR-8A ---
#define SENSOR_COUNT 8
#define PIN_EMITTER 7
QTRSensors qtr;
uint16_t sensorValues[SENSOR_COUNT];

// --- Velocidades de Maniobra ---
const int VELOCIDAD_BUSQUEDA = 90;
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

void accion_setMotores(int velocidadIzquierda, int velocidadDerecha) {
  digitalWrite(MOTOR_IZQ_IN1, HIGH);
  digitalWrite(MOTOR_IZQ_IN2, LOW);
  digitalWrite(MOTOR_DER_IN3, HIGH);
  digitalWrite(MOTOR_DER_IN4, LOW);

  velocidadIzquierda = constrain(velocidadIzquierda, 0, 255);
  velocidadDerecha = constrain(velocidadDerecha, 0, 255);

  analogWrite(MOTOR_IZQ_ENA, velocidadIzquierda);
  analogWrite(MOTOR_DER_ENB, velocidadDerecha);
}

void accion_Detener() {
  accion_setMotores(0, 0);
}

void accion_BuscarAdelante() {
  Serial.print("-> Accion: Buscando linea... (Intento ");
  Serial.print(lineaPerdidaContador);
  Serial.println(")");
  accion_setMotores(VELOCIDAD_BUSQUEDA, VELOCIDAD_BUSQUEDA);
  delay(TIEMPO_BUSQUEDA);
}

void esperarSiguientePaso() {
  Serial.println("\n--- PAUSA | Presiona 'n' y Enter para el siguiente paso ---");
  while (true) {
    if (Serial.available() > 0) {
      char input = Serial.read();
      if (input == 'n') {
        while(Serial.available() > 0) Serial.read(); 
        break;
      }
    }
  }
}

// ===== SECCIÓN 3: SETUP Y LOOP PRINCIPAL =====

void setup() {
  Serial.begin(9600);
  
  pinMode(MOTOR_IZQ_IN1, OUTPUT);
  pinMode(MOTOR_IZQ_IN2, OUTPUT);
  pinMode(MOTOR_IZQ_ENA, OUTPUT);
  pinMode(MOTOR_DER_IN3, OUTPUT);
  pinMode(MOTOR_DER_IN4, OUTPUT);
  pinMode(MOTOR_DER_ENB, OUTPUT);
  pinMode(STATUS_LED_PIN, OUTPUT);
  
  qtr.setTypeAnalog();
  qtr.setSensorPins((const uint8_t[]){A0, A1, A2, A3, A4, A5, A6, A7}, SENSOR_COUNT);
  qtr.setEmitterPin(PIN_EMITTER);
  
  Serial.println("--- INICIANDO CALIBRACION (10 SEGUNDOS) ---");
  digitalWrite(STATUS_LED_PIN, HIGH);

  for (int i = 0; i < 400; i++) {
    qtr.calibrate();
    delay(20);
  }

  digitalWrite(STATUS_LED_PIN, LOW);
  Serial.println("--- CALIBRACION FINALIZADA ---");
  Serial.println("Coloque el robot en la linea. Iniciando en 3 segundos...");
  delay(3000);
  Serial.println("¡En marcha!");
}


void loop() {
  int posicion = qtr.readLineBlack(sensorValues);
  
  if (posicion < 100 || posicion > 6900) {
      lineaPerdidaContador++;
      if (lineaPerdidaContador > MAXlineaPerdidaContador) {
        Serial.println("!!! PARADA DE EMERGENCIA: Linea no encontrada !!!");
        accion_Detener();
        while(true);
      } else {
        accion_BuscarAdelante();
      }
  } else {
    lineaPerdidaContador = 0;
    
    int error = posicion - 3500;
    float ajuste = Kp * error;
    int velocidadIzquierda = VELOCIDAD_MAXIMA + ajuste;
    int velocidadDerecha = VELOCIDAD_MAXIMA - ajuste;
  
    Serial.print("Pos: ");
    Serial.print(posicion);
    Serial.print(" | Error: ");
    Serial.print(error);
    Serial.print(" | Ajuste: ");
    Serial.print(ajuste);
    Serial.print(" | V_Izq: ");
    Serial.print(velocidadIzquierda);
    Serial.print(" | V_Der: ");
    Serial.println(velocidadDerecha);
  
    accion_setMotores(velocidadIzquierda, velocidadDerecha);
  }

  // ========= SECCIÓN DE DEBUG PASO A PASO =========
  #if MODO_DEBUG_PASO_A_PASO == 1
    delay(500);
    accion_Detener();
    esperarSiguientePaso();
  #endif
}
