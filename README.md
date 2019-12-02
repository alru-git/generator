# Generator

Überwachung des Notstromgenerators

Die Generator-Schaltung misst folgende Größen:
- Temperatur, Luftfeuchte und Luftdruck im Generatorgehäuse
- Spannung der Starterbatterie

Genutzte Technologien:
- Arduino (Controller für die Sensoren, https://store.arduino.cc/arduino-nano)
- Mysensors (Radio Gateway, https://www.mysensors.org/)
- Library: http://cactus.io/hookups/sensors/barometric/bme280/hookup-arduino-to-bme280-barometric-pressure-sensor
- Sensor: https://cdn.sparkfun.com/assets/learn_tutorials/4/1/9/BST-BME280_DS001-10.pdf
- Fhem (Hausautomation Controller, https://fhem.de/)

Inhalt:
- /docs: Dokumentation des Projektes
- /src: Code
