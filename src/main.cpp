#include <Arduino.h>
#include "DHT.h"
#include "secret.h"
#include <WiFi.h>
#include <ESPmDNS.h>
#include <PubSubClient.h>

// Définition du GPIO utilisé
//#define DHTPIN 21       // Broche GPIO21
#define DHTPIN 23       // Broche GPIO23
#define DHTTYPE DHT22   // Type de capteur

// Création de l'objet DHT
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);
  Serial.println("DHT22 test sur ESP32 WROOM-32");

  // Définir le hostname
  //WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE); // obligatoire pour certains firmwares
  //WiFi.setHostname("MonHostName"); 

// Connexion au WiFi
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

  dht.begin();

    // Démarrage du service mDNS
  if (!MDNS.begin("Thermodht")) {   // => Nom d’hôte : esp32wroom.local
    Serial.println("Erreur : mDNS n’a pas pu démarrer");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS actif : thermodht.local");

}

void loop() {
  // Délai minimum recommandé entre 2 lectures (~2s pour DHT22)
  delay(4000);

  float h = dht.readHumidity();
  float t = dht.readTemperature();        // °C
  float f = dht.readTemperature(true);    // °F

  // Vérification si la lecture est correcte
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Erreur de lecture du capteur DHT22 !");
    return;
  }

  // Calcul du Heat Index (indice de chaleur)
  float hif = dht.computeHeatIndex(f, h);
  float hic = dht.computeHeatIndex(t, h, false);

  // Affichage des valeurs
  Serial.print("Humidité: ");
  Serial.print(h);
  Serial.print(" %  Température: ");
  Serial.print(t);
  Serial.print(" °C / ");
  Serial.print(f);
  Serial.print(" °F  Indice chaleur: ");
  Serial.print(hic);
  Serial.println(" °C");
}
