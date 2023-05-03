
#include <Arduino.h>
#include <BluetoothSerial.h>
#include <ArduinoJson.h>

#include "SensorLib.h"
#include "Header.h"

TaskHandle_t Task1;

BluetoothSerial SerialBT;

StaticJsonDocument<1024> prev_JSON_Data; 
String JSON_String = "";
String prev_JSON_String = "";

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run 'make menuconfig' to and enable it
#endif

void BluetoothLED(uint8_t * Led_BT_count)
{
    if(*Led_BT_count>0)
    {
      LED_STATUS = 1;
      *Led_BT_count--;
    }
    else
    {
      LED_STATUS = 0;
    }

}


void codeForTask1(void * parameter)
{
  //Einmalige Ausführung vergleichbar zum "void Setup()" auf Core 0
  SerialBT.begin("ESP32_Sensormodul");
  Serial.println("Bluetooth is started now you can pair your Device");  

  //Endlosschleife für den Empfang von Bluetoothdaten
  for(;;)
  {
    //Wenn Bluetoothdaten verfügbar sind, versuche diese auszuwerten
    if(SerialBT.available())
    {
      //Bluetoothdaten als JSON-Datei interpretieren. Falls nicht möglich setzte error
      DeserializationError error = deserializeJson(Sensorlist::BT_JSON_Data, SerialBT);

      serializeJsonPretty(Sensorlist::BT_JSON_Data,JSON_String);


      //Wenn die Daten neu sind und noch nicht ausgewert, setzte "NEW_Bluetoothdata"
      if(!(prev_JSON_Data == Sensorlist::BT_JSON_Data))
      {
        if (error) {
          Serial.print("deserializeJson() failed: ");
          Serial.println(error.c_str());
        }
        else{
          Serial.print("deserializeJson() successed: ");

          //Gebe die empfangenen Daten auf dem Serial Monitor aus
          serializeJsonPretty(Sensorlist::BT_JSON_Data, Serial);
          Sensorlist::NEW_BT_Data = true;
          
        }
        //Empfangsbestätigung für den Sender
        SerialBT.println("Ready to send next");
        prev_JSON_String = JSON_String;
      }
    }
    //Pausiere den Task für 40 Ticks um Hintergrundtasks Zeit zur ausführung zu geben
    vTaskDelay(40);
  }
}


void Tasksetup()
{
  xTaskCreatePinnedToCore
  (
    codeForTask1,   //Funktion die Aufgerufen werden soll
    "Task1",        //Codewort des Tasks
    10000,          //Speicherreservierung auf dem Stack
    NULL,           //Task Input Parameter
    1,              //Priorität des Tasks 
    &Task1,         //Task Handle
    1               //CPU Core (0 ist der Standardcore)
  );
}

