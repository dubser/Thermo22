#include <Arduino.h>
#include "DHT.h"

// Définition du GPIO utilisé
#define DHTPIN 21       // Broche GPIO21
#define DHTTYPE DHT22   // Type de capteur

// Création de l'objet DHT
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  Serial.println("DHT22 test sur ESP32 WROOM-32");

  dht.begin();
}

void loop() {
  // Délai minimum recommandé entre 2 lectures (~2s pour DHT22)
  delay(2000);

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
