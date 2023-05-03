#include <Arduino.h>
#include "Header.h"
#include <Preferences.h>
#include "SensorLib.h"

// Include Libraries
#include <WiFi.h>
#include <esp_now.h>
#include <ArduinoJson.h>

// Define LED and pushbutton state booleans
bool buttonDown = false;
bool ledOn = false;
uint8_t Is_under_a_min_On = 0;
char prev_mac[18] = "";
uint16_t RangeTest_counter = 0;
bool LED_STATUS = false;


RTC_DATA_ATTR Sensorlist Sensorenliste;
Sensorlist Sensorenliste_tmp;

String MacAdresse;

RTC_DATA_ATTR int Seconds_since_on = 0;




void setup()
{
  //Set the CPU speed to 80Mhz to save Energy
  setCpuFrequencyMhz(80);

  // Set up Serial Monitor
  Serial.begin(115200);  

  //LED-Pin init
  pinMode(LED_INDICATOR, OUTPUT);
  digitalWrite(LED_INDICATOR, LOW);

  //GND-Pins init
  pinMode(GND_OUTPUT1, OUTPUT);
  digitalWrite(GND_OUTPUT1,LOW);
  pinMode(GND_OUTPUT2, OUTPUT);
  digitalWrite(GND_OUTPUT2,LOW);
  pinMode(GND_OUTPUT3, OUTPUT);
  digitalWrite(GND_OUTPUT3,LOW);
  pinMode(GND_OUTPUT4, OUTPUT);
  digitalWrite(GND_OUTPUT4,LOW);
  pinMode(GND_OUTPUT5, OUTPUT);
  digitalWrite(GND_OUTPUT5,LOW);

  //VCC-Pins init
  pinMode(VCC_OUTPUT1, OUTPUT);
  digitalWrite(VCC_OUTPUT1,HIGH);
  pinMode(VCC_OUTPUT2, OUTPUT);
  digitalWrite(VCC_OUTPUT2,HIGH);
  pinMode(VCC_OUTPUT3, OUTPUT);
  digitalWrite(VCC_OUTPUT3,HIGH);
  pinMode(VCC_OUTPUT4, OUTPUT);
  digitalWrite(VCC_OUTPUT4,HIGH);

  //Verzögerung um eine korrekte Batteriespannungsmessung zu ermöglichen.
  vTaskDelay(100); 
  
  //Batteriespannung kontrollieren. 
  Check_U_Batt(Seconds_since_on);

  //ESP-NOW initialisieren und eigene Mac-Adresse abspeicher
  MacAdresse = Initial_ESP_NOW();
  
  //Deep Sleep Intervalle initialisieren
  DeepSleep_init_time(DEEPSLEEPTIME);
}

void loop()
{
  //Setup welches nur im Fall des ersten Bootens ausgeführt wird
  if(Seconds_since_on < BT_ONTIME_FIRST_BOOT)
  {
    //CPU Frequenz auf 240MHz setzen für zusätzliche Stabilität während des Kopplungsvorgangs
    setCpuFrequencyMhz(240);

    //Timer Interupts initialisieren
    Timersetup();

    //Aktiviere den 2. CPU Core mit der Bluetooth Funktion
    Tasksetup();

    //Lade die MacAdresse aus dem EEPROM 
    Sensorlist tmp_Sensorlist;
    tmp_Sensorlist.Load_Settings();

    if(tmp_Sensorlist.MacAdresseTarget.length()>8 
    && tmp_Sensorlist.Sensor_Counter>0)
    {
      Sensorenliste.MacAdresseTarget = tmp_Sensorlist.MacAdresseTarget;
    }
  }

  //While Schleife welche die ersten 30 Sekunden ausgeführt wird.
  while(Seconds_since_on < BT_ONTIME_FIRST_BOOT)
  {
    //Überprüfe ob BT Daten vorhanden sind
    if(Sensorlist::NEW_BT_Data==true)
    {
      Sensorenliste.BT_Config_Data();
      Sensorlist::NEW_BT_Data = false;
      Sensorlist::BT_Configuration_loaded = true;
    }

    //Falls bei diesem oder einem vorherigen Loop-Iteration noch keine BT-Daten empfangen wurde 
    //und ESP-NOW Daten verfügbar sind übernehme diese
    if(Sensorlist::NEW_ESPNOW_Data==true && Sensorlist::BT_Configuration_loaded==false)
    {
      Sensorlist::NEW_ESPNOW_Data = false;
      Sensorlist::ESPNOW_Configuration_loaded = true;
      Sensorenliste.Convert_JSON_TO_DATA_ESPNOW();       
    }

    //Wenn der aktuelle Master ein Range-Signal sendet, blinke 2x
    if(Sensorlist::RangeTest_status == true)
    {

      digitalWrite(LED_INDICATOR_BUILTIN, LOW);
      digitalWrite(LED_INDICATOR, HIGH);
      vTaskDelay(RANGETEST_LED_CYCLE);
      digitalWrite(LED_INDICATOR_BUILTIN, HIGH);
      digitalWrite(LED_INDICATOR, LOW);
      vTaskDelay(RANGETEST_LED_CYCLE);
      Serial.println("Recieved sth.");

      Sensorlist::RangeTest_status = false;
    }

    
    //Ausführung jede Sekunde
    if(Second_flag==true)
    {
      Second_flag = false;
      Seconds_since_on++;
    }

    //Taskdelay von 5ms
    vTaskDelay(5);
  }

//Überprüfe ob BT/ESP-NOW Daten vorhanden sind, falls nein überneheme Daten aus dem EEPROM
  if(Sensorlist::BT_Configuration_loaded == true && Sensorlist::ESPNOW_Configuration_loaded == false)
  {
    //Speichere die Daten im EEPROM
    Sensorenliste.Save_Settings();
    Serial.println("Saved");
  }
  else if(Sensorlist::BT_Configuration_loaded == false && Sensorlist::ESPNOW_Configuration_loaded == true)
  {
    //Ergänze die Sensorliste um die voreingestellten Sensortypen
    Serial.println("NEW_SENSOR123");
    Sensorenliste.Add_Sensor(VALVE_PORT_OF_THIS_BOX,ANALOG_READ1);
    Sensorenliste.Add_Sensor(VALVE_PORT_OF_THIS_BOX,ANALOG_READ2);

    //Speichere die Daten im EEPROM
    Sensorenliste.Save_Settings();
    Serial.println("Saved");
  }
  else if(Sensorlist::BT_Configuration_loaded == false && Sensorlist::ESPNOW_Configuration_loaded == false)
  {
    //Lade Daten aus dem EEPROM
    Sensorenliste.Load_Settings();
    Serial.println("Loaded");
  }
  
  //Lese alle Feuchtigkeitssensoren aus
  Sensorenliste.Read_all_Sensors();
  //Gebe einmal alle Daten der Sensorliste zur kontrolle im Seriellen Monitor aus
  Sensorenliste.Print_Sensors();
  Serial.println(Sensorenliste.JSON_to_String(VALVE_PORT_OF_THIS_BOX, analogRead(U_BATT)));

  //Messe die aktuelle Temperatur
  Sensorenliste.Temperature = getTemperature();

  //Sende 2x alle Daten an den Master-Schaltschrank um eine mögliche Fehlsendung zu reduzieren
  broadcast(Sensorenliste.JSON_to_String(VALVE_PORT_OF_THIS_BOX, analogRead(U_BATT)));
  vTaskDelay(1000);
  broadcast(Sensorenliste.JSON_to_String(VALVE_PORT_OF_THIS_BOX, analogRead(U_BATT)));

  //Versetze den ESP32 in den Deep Sleep Modus
  Going_to_Sleep();
}