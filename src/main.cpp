//v20250927 Ajout de monitoring de batterie
// v20250923 Cette version fonctionne  4 heures avec  une transmission 
// au 5 sec sans Deep sleep à test.mosquitto.org et une batterie 9V alcaline.
// v20250924 Version  avec Deep sleep et xmit 60 sec.
// v20250928 Calcul de la tension de batterie xmit 10sec
// v20251001 Complet et vériié Xmit 1/min, température ajustée -2°C

#include <Arduino.h>
#include "DHT.h"
#include "secret.h"
#include <WiFi.h>
#include <ESPmDNS.h>
#include <PubSubClient.h>

// Définition GPIO pour le monitoring de la batterie
const int adcPin = 32;  // Broche analogique à lire (GPIO32)
const float VCC = 3.3;  // Tension d'alim
const int ADC_MAX = 1023; // 10 bits -> 0..1023

// Définition GPIO
//#define DHTPIN 23   // Broche GPIO23 défectueuse sur un Esp32
//GPIO pleinement utilisabes 13, 14, 16, 17, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33
#define DHTPIN 21     // Broche GPIO21

// Type de capteur
#define DHTTYPE DHT22

// Création de l'objet DHT
DHT dht(DHTPIN, DHTTYPE);

// MQTT 
const char* mqtt_server = "test.mosquitto.org";  // adresse du broker
WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  WiFi.begin(wifiId, pasw);
  Serial.print("Connexion au WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connecté !");
  Serial.println(WiFi.localIP());
  // Adresse MAC en mode station (STA)
  Serial.print("MAC (STA) : ");
  Serial.println(WiFi.macAddress());
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect("ESP32_thdht0")) {
      // Ici on pourrait faire client.subscribe("test/topic");
      // MAIS ce n’est pas obligatoire pour publier
    } else {
      delay(500);
    }
  }
}

void dowork(){
  delay(1000);

  // Lecture de la tension de la batterie
  int raw = analogRead(adcPin);          // Lecture ADC brute
  float voltage = (raw / float(ADC_MAX)) * VCC;  // Conversion en volts
 
  // Lire le dht22
  float h = dht.readHumidity();
  float t = dht.readTemperature();        // °C
  t=t-2.0; // Ajustement de +2 °C pour concordance avec un autre capteur
  float f = dht.readTemperature(true);    // °F

  // Vérification si la lecture DHT est correcte
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Erreur de lecture du capteur DHT22 !");
    return;
  }

  // Affichage des valeurs temp et humidité
  Serial.print("Humidité: ");
  Serial.print(h);
  Serial.print(" %  Température: ");
  Serial.print(t);// Ajustement de +2 °C pour concordance avec un autre capteur
  Serial.print(" °C  ");
  Serial.println();

  // Affichage de la tension de la batterie

  float v9 = 4.36 * voltage; // Ajuster selon le diviseur de tension
  Serial.print("Raw: ");
  Serial.print(raw);
  Serial.print("  |  Voltage: ");
  Serial.print(voltage, 3); // 3 décimales
  Serial.print("  |  Volt/9v: ");
  Serial.println(v9, 3); // 3 décimales


// Publication des valeurs sur MQTT mqtt_server

if (!client.connected()) {
    reconnect();
  }
  delay(1000);
  
  // Conversion de float en string
  char ts[10]; char hs[10]; char v9s[10];
  dtostrf(t, 6, 2, ts);  // largeur=6, décimales=2
  dtostrf(h, 6, 2, hs);  // largeur=6, décimales=2
  dtostrf(v9, 6, 2, v9s);  // largeur=6, décimales=2

    client.publish("thdht0/temp", ts, true);
    delay(500);
    client.publish("thdht0/hum", hs, true);
    delay(500);
    // Publier la tension de la batterie
    client.publish("thdht0/batt", v9s, true);
    Serial.println("Données publiées vers MQTT");
}

void setup() {
  analogReadResolution(10); // On force 10 bits
  delay(2000); // Attente de la stabilisation du système
  Serial.begin(9600);
  Serial.println("");
  Serial.println("Éveil du ESP32 WROOM-32");

  setup_wifi(); // Connexion au WiFi
  dht.begin();  // Initialisation du capteur DHT22
  client.setServer(mqtt_server, 1883);// InitT mqtt

// Démarrage du service mDNS
  if (!MDNS.begin("thdht")) {   // => Nom d’hôte : thdht.local
    Serial.println("Erreur : mDNS n’a pas pu démarrer");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS actif : thdht.local");

  // Activer le réveil par timer
  esp_sleep_enable_timer_wakeup(60 * 1000000);
//    esp_sleep_enable_timer_wakeup(1 * 1000000);

  dowork();  // Effectue la job

  // Passage en deep sleep
  Serial.println("Passage en deep sleep...\n");
  Serial.flush(); // Attente de la fin de l’envoi des données série
  esp_deep_sleep_start();
  
}

void loop() {
  
} 
