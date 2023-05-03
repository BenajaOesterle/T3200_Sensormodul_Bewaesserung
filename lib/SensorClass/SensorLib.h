#ifndef SensorLib_h
#define SensorLib_h

#include <WiFi.h>
#include <Arduino.h>
#include <BluetoothSerial.h>
#include <Preferences.h>
#include <string.h>
#include <string>
#include <ArduinoJson.h>
#include <esp_now.h>

#define SENSORQUANTITY 8                //Maximale Anzahl an konfigurierbaren Sensoren
#define READINGQUANTITY 8               //Anzahl an Messwerten die Pro Sensor abgespeichert werden können
#define READINGS_PER_MESSUREMENT 100    //Anzahl an Messungen, welche gemittelt werden um Fehler zu minimieren
#define DEEPSLEEPTIME 900               //Sekunden, die der ESP32 im DeepSleep verbringt.

class Sensor{
    private:
        uint8_t curr_readinglist = 0;                   //Anzahl an Messwerten in der Messwertliste             

    public: 
        uint8_t Valve_Port = 0;                         //0 = nicht verwendet nur bei Mehreren Ventilen pro Sensorbox erforderlich
        uint8_t Sensor_Port = 0;                        //Hardwareanschluss des Sensors
        uint16_t Sensor_readinglist[READINGQUANTITY];   //Liste mit den Messwerten
        void Add_new_Value(uint16_t reading);           //Füge einen neuen Messwert hinzu
        void Init_readinglist();                        //Initialisiere alle Messwertpositionen mit 0
        void Print_Sensor();                            //Gebe alle Sensordaten im Seriellen Monitor aus
        void Read_Sensor();                             //Lese Senor aus und füge den Messwert der Liste hinzu
        uint16_t Average_Reading();                     //Ermittle einen Mittelwert aller Messwerte in der Liste
};


class Sensorlist{

    public:
        static StaticJsonDocument<1024> BT_JSON_Data;                       //Speichert JSON Bluetooth Daten hier temporär ab
        static StaticJsonDocument<1024> ESPNOW_JSON_Data;                   //Speichert JSON ESP-NOW Daten hier temporär ab
        static bool NEW_BT_Data;                                            //Neue Bluetooth Datei wurde empfangen 
        static bool NEW_ESPNOW_Data;                                        //Neue ESP-NOW Datei wurde empfangen 
        static bool BT_Configuration_loaded;                                //Die Bluetoothkonfiguration wurde geladen
        static bool ESPNOW_Configuration_loaded;                            //Die ESP-NOW-Konfiguration wurde geladen
        static bool RangeTest_status;                                       //Es wurde ein Schaltschrank ESP32 Master empfangen

        float Temperature = 0;                                              //Die Aktuelle Temperatur 

        uint8_t Valve_of_this_box = 0;                                      //Wenn die Box nur Messwerte für ein Ventil liefert, wird dieses hier abgespeichert.

        String MacAdresseTarget;                                            //Mac Adresse des Schaltschranks
        uint8_t Sensor_Counter = 0;                                         //Anzahl an Sensoren in der Sensorliste
        Sensor SensorListe[SENSORQUANTITY];                                 //Sensorliste
        void Add_Sensor(uint8_t Valve_Port = 0, uint8_t Sensor_Port = 0);   //Sensor hinzufügen 
        void Print_Sensors();                                               //Gebe alle Sensoren im Seriellen Monitor aus
        void Load_Settings();                                               //Lade die letzte verwendete Liste aus dem EEPROM
        void Save_Settings();                                               //Speichere die aktuelle Liste im EEPROM
        void Read_all_Sensors();                                            //Lese alle Sensoren einmal aus und speichere deren Werte ab.
        String JSON_to_String(uint8_t Valve_of_this_box, uint16_t U_Batt);  //Formatiere die aktuellen Daten aus der Sensorliste in ein übertragungsfähiges JSON Format.
        void BT_Config_Data();                                              //Werte empfangene Bluetoothdaten aus
        void Convert_JSON_TO_DATA_ESPNOW();                                 //Werte per ESP-NOW empfangene Daten aus
        uint16_t Combine_Readings();                                        //Kombiniere Messungen mehrerer Sensoren wenn Daten für nur ein Ventil gesammelt werden
};


#endif