/**@file Mapowanie.ino */

#include "crc16.h"

//!  Klasa czujnika ultradźwiękowego
/*!
  Klasa czujnika posiada pola przechowujące informacje o ID czujnika
  oraz pinów mikrokontrolera używanych przez czujnika
*/
class Sensor {
public:
    uint8_t SENSOR_ID;  /*!< ID czujnika*/
    uint8_t TRIG_PIN;   /*!< Trigger PIN*/
    uint8_t ECHO_PIN;   /*!< Echo PIN*/
    //! Konstruktor.
    /*!
        Konstruktor tworzący instancję czujnika
        \param[in] ID czujnika
        \param[in] Trigger PIN
        \param[in] Echo PIN
    */
    Sensor(uint8_t SENSOR_ID, uint8_t TRIG_PIN, uint8_t ECHO_PIN);
};

Sensor::Sensor(uint8_t SENSOR_ID, uint8_t TRIG_PIN, uint8_t ECHO_PIN) {
    Sensor::SENSOR_ID = SENSOR_ID;
    Sensor::TRIG_PIN = TRIG_PIN;
    Sensor::ECHO_PIN = ECHO_PIN;
}

Sensor sensorList[] = { Sensor(0, 4, 5), Sensor(1, 8, 9), Sensor(2, 12, 13) };  //!< Lista przechowująca instancje czujników

CRC16 crc; //!< Instancja obiektu za pomocą którego wyznaczana jest suma kontrolna

unsigned long previousTimeReadSensor = 0;
unsigned long previousTimeStartLoop = 0;
byte incomingData = 0;
long duration, distanceCm;
char numstr[21];
String data = "";
bool startMapping = false;

/*!
    Funkcja zwracająca długość pulsu z danego czujnika
    \param[in] Trigger PIN
    \param[in] Echo PIN
    \param[out] Długość pulsu
*/
long readFromSensor(int TRIG_PIN, int ECHO_PIN) {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    return pulseIn(ECHO_PIN, HIGH);
}

void readFromSerial() {
    if (Serial.available() > 0) {
        // read the incoming byte:
        incomingData = Serial.read();
        if (incomingData == 49) {
            Serial.println(incomingData);
            startMapping = true;
        }
        else if (incomingData == 48) {
            Serial.println(incomingData);
            startMapping = false;
        }
    }
}

/*!
    Główna funkcja ustawiająca wartości potrzebne do poprawnego działania
    programu. Tryb działania PINów Trigger każdego z czujników jest ustawiany
    jako wyjście a PINów Echo jako wejście.
    Inicjalizacja portu seryjnego z zadaną szybkością transmisji.
    Ustawienie wielomianu CRC.
*/
void setup() {
    for (Sensor sensor : sensorList) {
        pinMode(sensor.TRIG_PIN, OUTPUT);
        pinMode(sensor.ECHO_PIN, INPUT);
    }
    Serial.begin(9600);

}

/*!
    Główna pętla programu.
*/
void loop() {

    readFromSerial();

    if (millis() - previousTimeStartLoop >= 1000UL && startMapping) {
        previousTimeStartLoop = millis();

        data = "";

        for (int i = 0; i < sizeof(sensorList) / sizeof(int) - 1 ;) {

          readFromSerial();
        
          if (millis() - previousTimeReadSensor >= 200UL) {
            previousTimeReadSensor = millis();

            duration = readFromSensor(sensorList[i].TRIG_PIN, sensorList[i].ECHO_PIN);
            distanceCm = duration / 29.1 / 2;
            
            if (distanceCm <= 0) {
                Serial.println("Out of range");
            } else {
                sprintf(numstr, "%d %lu ", sensorList[i].SENSOR_ID, distanceCm);
                data = data + numstr;
            }
            i++;
          }
        }

        uint16_t calculated_crc = crc.calculateCRC16(data.c_str(), sizeof(data));
        sprintf(numstr, "%02X", calculated_crc );
        data = data + numstr;
        Serial.println(data);
    }
}
