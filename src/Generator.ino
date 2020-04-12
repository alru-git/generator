/**
 * 
 * DESCRIPTION
 * 
 * It communicates over the I2C protocol.
 * 
 * Jetzt mit der cactus Library
 * 
 * Connect your sensor's powerlines, and connect your sensor to the SDA and SCL pins of your board. 
 * On Arduino Nano SDA is pin A4, and SCL is pin A5.
 * On the Ardunio Mega and Due the SDA in pin 20, and the SCL is pin 21.
 * On the Arduino Leonardo and Pro Micro 2 the SDA in pin 2, and the SCL is pin 3.
 * 
 *
 * The BME280 datasheet: https://cdn.sparkfun.com/assets/learn_tutorials/4/1/9/BST-BME280_DS001-10.pdf
 *
 *Der Nano benutzt den "Old Bootloader"
 *
 *ver 0.3 : erste stabile Version
 *ver 0.4 : Mit Spannungsmessung, ACK der Sp.-Messung wird wie bei der Stockwaage geprüft
 *ver 0.5 : An MSG2          
 *ver 0.7 : Test mit Batteriemessung, RF MAX und an MSG1
 *ver 0.8 : Prod. Test an MSG2 mit 1h Loop
 *ver 0.9 : Mit Korrekturwert (siehe Spannungs-Formel) im Vergleich zum Messgerät, erster prod Einsatz
 *ver 1.0 : Neuer Korrekturwert (0.975) in der Spannungs-Formel, da das Messergebnis zu niedrig war
 *ver 1.1 : Neuer Korrekturwert (0.999) in der Spannungs-Formel, da das Messergebnis zu niedrig war
 *
 * 
 *
 */

// Enable debug prints to serial monitor
#define MY_DEBUG

// Enable and select radio type attached
#define MY_RADIO_RF24

// MSG1 (Frequenz 2496 MHz)
// #define MY_RF24_CHANNEL 96
// MSG2 (Frequenz 2506 MHz)
#define MY_RF24_CHANNEL 106

// RF Power
#define MY_RF24_PA_LEVEL RF24_PA_MAX

// Optional: Define Node ID
#define MY_NODE_ID 123
// Node 0: MSG1 oder MSG2
#define MY_PARENT_NODE_ID 0
#define MY_PARENT_NODE_IS_STATIC

// LIBRARIES
#include <MySensors.h>
#include <Wire.h>
#include "cactus_io_BME280_I2C.h"

BME280_I2C bme(0x76);                 // I2C using address 0x76, wurde in der cactus_io_BME280_I2C.h geändert

unsigned long SLEEP_TIME = 3600000;   // Laufzeit der Loop 1h = 3600000
unsigned long WAIT_Time1 = 1000;      // 1s, Wartezeit, zum Empfang von ACK
unsigned long WAIT_Time10 = 10000;    // 10s, Wartezeit, bis der 2. Sendeversuch erfolgt

// for MySensors. Within this node each sensortype should have its own ID number.
#define TEMP_CHILD_ID 0                
#define HUM_CHILD_ID 1
#define BARO_CHILD_ID 2

// MYSENSORS COMMUNICATION VARIABLES
MyMessage temperatureMsg(TEMP_CHILD_ID, V_TEMP);
MyMessage humidityMsg(HUM_CHILD_ID, V_HUM);
MyMessage pressureMsg(BARO_CHILD_ID, V_PRESSURE);

// --- Spannungsmessung
  int BATTERY_SENSE_PIN = A0;      // select the input pin for the battery sense point
  MyMessage msgV(0,V_VOLTAGE);
// --- Ende

void setup() {
  Serial.begin(115200);

  bme.begin();
  bme.setTempCal(-1);

  // DEFAULT nutzt beim 5V Nano die VCC als Ref.-Spg.(=10bit also 1024 Steps)
  // INTERNAL würde 1.1V als Ref. Spg. nutzen
  analogReference(DEFAULT);
}

void presentation()  {
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("Generator", "1.1");

  // Tell the MySensors gateway what kind of sensors this node has, and what their ID's on the node are
  present(BARO_CHILD_ID, S_BARO);
  present(TEMP_CHILD_ID, S_TEMP);
  present(HUM_CHILD_ID, S_HUM);

  // --- Spannungsmessung
  present(0, S_MULTIMETER);  

}

void loop() {
  // Update vom heartbeat
  sendHeartbeat();

  bme.readSensor();

 float temperature = bme.getTemperature_C();
 float humidity = bme.getHumidity();
 float pressure_local = bme.getPressure_MB();
 float pressure = pressure_local + 8.6;        // Differenz aus der Fhem Berechnung genommen

 // Serielle Ausgabe
 #ifdef MY_DEBUG
   Serial.print("BME280 - Temperature = ");
   Serial.print(temperature);
   Serial.println(" °C");
   Serial.print("BME280 - Humidity = ");
   Serial.print(humidity);
   Serial.println(" %");
   Serial.print("BME280 - Pressure = ");
   Serial.print(pressure);
   Serial.println(" hPa");
 #endif

 // Senden zum Gateway
 send(temperatureMsg.set(temperature, 1));
 send(humidityMsg.set(humidity, 1));
 send(pressureMsg.set(pressure, 1));

 // --- Spannungsmessung
      int sensorValue = analogRead(BATTERY_SENSE_PIN);
      
      #ifdef MY_DEBUG
        Serial.print(F("A0 Spg.-Wert "));
        Serial.println(sensorValue);
      #endif

     // Berechnung der Batteriespg. am Spg.-Teiler (1M zu 0,47M)
     // Formel = (10bit-Wert) * (Ref.-Spg.) / (10bit) / (Spg.-Teiler-Wert) * (Korrekturwert_zum_Messgerät)
     float batteryV = sensorValue * 5.0 /1024.0 / 0.3197 * 0.999; 
      
     #ifdef MY_DEBUG
        Serial.print("Battery Voltage: ");
        Serial.print(batteryV);
        Serial.println(" V");
     #endif

     // Nachkommastellen müssen begrenzt werden
     send(msgV.set(batteryV, 2));
      wait(WAIT_Time1);
      if (send(msgV.set(batteryV, 2), true)){       
        #ifdef MY_DEBUG
          Serial.println("ACK war OK");
        #endif
         }
      else {
        wait(WAIT_Time10);
        send(msgV.set(batteryV, 2));
        }
  // --- Ende


 sleep (SLEEP_TIME);
}

