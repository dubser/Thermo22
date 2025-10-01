# Thermo22
Thermomètre DHT22 avec Esp32 Wroom et
Connexion web élémentaire utilisant 
le protocole de connection mdns.
Les ports sécuritaires GPIO 13, 14, 16, 17, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33
Ajout d'une structure pour invisibiliser les passwd sur GitHub et GPIO21 => GPIO23
07 sept 2025 On est a 
Flash: 57.5% (used 753185 bytes from 1310720 sur HARDWARE: ESP32 240MHz, 320KB RAM, 4MB Flash

20 sept 2025 Après avoir ajouté la logique de post a MQTT
RAM:   [=         ]  14.0% (used 45832 bytes from 327680 bytes)
Flash: [======    ]  59.1% (used 775253 bytes from 1310720 bytes)
+ 2 % de flash
v20250923 Cette version Fonctionne  4 heures avec  une transmission 
au 5 sec sans Deep sleep à test.mosquitto.org et une batterie 9V alcaline.
v20250924 Version  avec Deep sleep et xmit 60 sec.
v20250927 Ajout de monitoring de batterie.
v20251001 Complet et vériié Xmit 1/min, température ajustée -2°C
