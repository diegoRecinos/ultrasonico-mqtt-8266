#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

#define TRIG_PIN D6
#define ECHO_PIN D7

/************************* WiFi Access Point *********************************/
#define WLAN_SSID "estelarlol"
#define WLAN_PASS "estelarloleldenring"

/************************* Adafruit.io Setup *********************************/
#define AIO_SERVER "io.adafruit.com"
#define AIO_SERVERPORT 1883
#define AIO_USERNAME "recinos000"
#define AIO_KEY "****" // Reemplaza con tu clave real

/************ Global State (no necesitas cambiar esto!) ******************/
WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_USERNAME, AIO_KEY);

/****************************** Feed para Suscribirse ***************************************/
// Configura un feed llamado 'FeedTest' para recibir datos.
Adafruit_MQTT_Subscribe ultrasonicoFeed = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/ultrasonico");

// Declara un feed de publicación para la temperatura del LM35
Adafruit_MQTT_Publish ultrasonicoFeed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/ultrasonico");

/* **************** Declaración de variables globales *******************/
const int ledPin = D0;

/*************************** Sketch Code ************************************/

/*********************************************** SetUp *****************************/
void setup() {
  Serial.begin(115200);
  delay(100);
  pinMode(ledPin, OUTPUT);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  Serial.println("\nIniciando configuracion...");

  // Conexión a la red Wi-Fi
  Serial.print("Conectando a WiFi: ");
  Serial.print(WLAN_SSID);
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado! ");

  // Configura la función de callback para el feed
  ultrasonicoFeed.setCallback(handleMessage);

  // Suscríbete al feed
  mqtt.subscribe(&ultrasonicoFeed);
}

//***************************************************** Loop ********************************/
void loop() {
  static uint32_t tempTimer = millis();
  if (millis() - tempTimer > 10000) { // Enviar la lectura cada 30 segundos
    tempTimer = millis();

 // Limpiar el pin Trigger
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  
  // Generar pulso de 10 µs en el pin Trigger
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Leer el tiempo que tarda el pulso en volver (en microsegundos)
  long duracion = pulseIn(ECHO_PIN, HIGH);
  
  // Calcular la distancia: (duracion * velocidad_del_sonido) / 2
  // Velocidad del sonido ≈ 0.0343 cm/µs
  float distancia_cm = duracion * 0.0343 / 2;

  Serial.println("\n--- DATOS DEL HC-SR04 (Distancia) ---");
  Serial.print("Distancia: ");
  Serial.print(distancia_cm);
  Serial.println(" cm");
  Serial.println("======================================");

  delay(1000);
    // Publicar la temperatura al feed de Adafruit.io
    if (!ultrasonicoFeed.publish(distancia_cm)) {
      Serial.println("Error al publicar la distancia.");
    } else {
      Serial.println("distancia publicada con exito!");
    }
  }

  // Asegura la conexión a MQTT
  MQTT_connect();

  // Procesa los paquetes entrantes del MQTT (mensajes de Adafruit.io)
  mqtt.processPackets(100);

  // Mantiene la conexión viva
  if (!mqtt.ping()) {
    mqtt.disconnect();
  }
}

// Función para conectar y reconectar al servidor MQTT
void MQTT_connect() {
  int8_t ret;
  if (mqtt.connected()) {
    return;
  }
  Serial.print("Conectando a MQTT... ");
  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) {
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Reintentando conexión MQTT en 10 segundos...");
    mqtt.disconnect();
    delay(10000);
    retries--;
    if (retries == 0) {
      while (1);
    }
  }
  Serial.println("MQTT Conectado! ");
}
