//v20250927   Ajout de monitoring de batterie
// v20250923  Cette version fonctionne  4 heures avec  une transmission 
// au 5 sec sans Deep sleep à test.mosquitto.org et une batterie 9V alcaline.
// v20250924  Version  avec Deep sleep et xmit 60 sec.
// v20250928  Calcul de la tension de batterie xmit 10sec
// v20251001  Complet et vériié Xmit 1/min, température ajustée -2°C
// v20260119  Serveur MQTT local, Eveil 5 sec pour debug.
// v20260127  Ajustement diviseur de tension, suppression monitoring batterie & correction timeout WiFi
// v20260102  Ajouter wakeupCount et wifiFault.
// v20260102  Enlever mDNS.
// v20260202  Correction publication wifiCount & delay entre publications 5 MIN
// v20260331a Changement Ip statique. 
// v20260331  Alout de stayOn pour rester allumé pour OTA, sinon deep sleep

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
RTC_DATA_ATTR int wakeupCount = 0;
RTC_DATA_ATTR int wifiFault = 0;
RTC_DATA_ATTR int wifiCount = 0;
const bool stayOn = true; // true pour rester allumé pour OTA, false pour deep sleep après setup()

// Définition GPIO
//#define DHTPIN 23   // Broche GPIO23 défectueuse sur un Esp32
//GPIO pleinement utilisabes 13, 14, 16, 17, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33
#define DHTPIN 21     // Broche GPIO21

// Type de capteur
#define DHTTYPE DHT22

// Création de l'objet DHT
DHT dht(DHTPIN, DHTTYPE);

// ⚙️ Configuration IP statique  v20260331a
IPAddress local_IP(192, 168, 1, 201);   // IP statique souhaitée pour l'ESP32
IPAddress gateway(192, 168, 1, 1);     // Routeur
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(8, 8, 4, 4);

// MQTT 
//const char* mqtt_server = "test.mosquitto.org";  // adresse du broker
const char* mqtt_server = "192.168.1.9";  // adresse du broker
WiFiClient espClient;
PubSubClient client(espClient);


bool setup_wifi(uint32_t timeout_ms = 15000) {

  // ⚡️ Configurer l'IP statique
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("Erreur de configuration IP statique !");
  }

  // ⚡️ Configurer l'IP statique
  WiFi.begin(wifiId, pasw);
  Serial.print("Connexion WiFi");

  uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (millis() - start > timeout_ms) {
      Serial.println("\nWiFi non connecté (timeout)");
      wifiFault++;
      Serial.print("wifiFault = ");
      Serial.println(wifiFault);
      return false;
    }
  }

  Serial.println("\nWiFi connecté !");
  Serial.println(WiFi.localIP());
  wifiCount++;

  // Adresse MAC en mode station (STA)
  Serial.print("MAC (STA) : ");
  Serial.println(WiFi.macAddress());
    return true; 
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
// sd  delay(1000);wakeupCounteveii

  // Lecture de la tension de la batterie
  int raw = analogRead(adcPin);          // Lecture ADC brute
  float voltage = ((raw / float(ADC_MAX)) * VCC);  // Conversion en volts
 
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

  //float v9 = (4.36 * voltage)+.4; // Ajuster selon le diviseur de tension
  float v9 = (4.85 * voltage); // Ajuster selon le diviseur;
  Serial.print(raw);
  Serial.print("  |  Voltage: ");
  Serial.print(voltage, 3); // 3 décimales
  Serial.print("  |  Volt/9v: ");
  Serial.println(v9, 3); // 3 décimales


// Publication des valeurs sur MQTT mqtt_server

if (!client.connected()) {
    reconnect();
  }
  // sd delay(1000);
  
  // Conversion de float en string
  char ts[10]; char hs[10]; char v9s[10];
  dtostrf(t, 6, 2, ts);  // largeur=6, décimales=2
  dtostrf(h, 6, 2, hs);  // largeur=6, décimales=2
  dtostrf(v9, 6, 2, v9s);  // largeur=6, décimales=2

// Publication au serveur MQTT
    client.publish("thdht0/temp", ts, true);
    delay(500);
    client.publish("thdht0/hum", hs, true);
    delay(500);
    // Publier la tension de la batterie
    client.publish("thdht0/batt", v9s, true);
   
   // Publier les compteurs wakeupCount et wifiFault
    delay(500);
    client.publish("thdht0/wakeupCount", String(wakeupCount).c_str(), true);
    delay(500); 
    client.publish("thdht0/wifiFault", String(wifiFault).c_str(), true);  
    delay(500);
    client.publish("thdht0/wifiCount", String(wifiCount).c_str(), true);

    Serial.println("Données publiées vers MQTT");
}

void setup() {
  analogReadResolution(10); // On force 10 bits
  delay(2000); // Attente de la stabilisation du système
  Serial.begin(9600);
  Serial.println("");
  Serial.println("Éveil du ESP32 WROOM-32");
  wakeupCount++;
  Serial.print("wakeupCount = ");
  Serial.println(wakeupCount);

 // setup_wifi(); // Connexion au WiFi

bool wifiOK = setup_wifi(); // Connexion au WiFi

if (!wifiOK) {
  Serial.println("WiFi indisponible → deep sleep");
  esp_sleep_enable_timer_wakeup(60 * 1000000);
  esp_deep_sleep_start();
}


  dht.begin();  // Initialisation du capteur DHT22
  client.setServer(mqtt_server, 1883);// InitT mqtt

// Démarrage du service mDNS
/* if (!MDNS.begin("thdht0")) {   // => Nom d’hôte : thdht0.local
    Serial.println("Erreur : mDNS n’a pas pu démarrer");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS actif : thdht0.local");
*/ 

  // Activer le réveil par timer
   if (!(stayOn)) {
  esp_sleep_enable_timer_wakeup(300* 1000000); //  300 seconde   
  // esp_sleep_enable_timer_wakeup(5 * 1000000); // 5 seconde pour debug
  } 

  dowork();  // Effectue la job

  // Passage en deep sleep
  if (!(stayOn)) {
  Serial.println("Passage en deep sleep...\n");
  Serial.flush(); // Attente de la fin de l’envoi des données série
  esp_deep_sleep_start();
  } 
  
}

void loop() {
  delay(5000); // Ne fait rien, le travail est fait dans setup() et on est en deep sleep 
  Serial.println("Bouclage dans loop...\n");
  dowork();  // Effectue la job
} 
