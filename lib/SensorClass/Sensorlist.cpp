#include <Arduino.h>
#include <Preferences.h>
#include <string.h>
#include <string>
#include <ArduinoJson.h>

#include "SensorLib.h"

StaticJsonDocument<1024> Sensorlist::BT_JSON_Data;
StaticJsonDocument<1024> Sensorlist::ESPNOW_JSON_Data;
bool Sensorlist::NEW_BT_Data = false;
bool Sensorlist::NEW_ESPNOW_Data = false;
bool Sensorlist::BT_Configuration_loaded = false;
bool Sensorlist::ESPNOW_Configuration_loaded = false;
bool Sensorlist::RangeTest_status = false;


void Sensorlist::Add_Sensor(uint8_t Valve_Port, uint8_t Sensor_Port){
    if(Sensor_Counter<SENSORQUANTITY)
    {
        Sensor_Counter++;
        SensorListe[Sensor_Counter].Valve_Port = Valve_Port;
        SensorListe[Sensor_Counter].Sensor_Port = Sensor_Port;
        SensorListe[Sensor_Counter].Init_readinglist();
        Serial.printf("Sensor Nr%d angelegt\n", Sensor_Counter);
    }
    else
        Serial.println("Error to many Sensors");
}

void Sensorlist::Print_Sensors(){
    
    Serial.println("Print Sensors-------------------------");

    for(int i = 1; i<=Sensor_Counter; i++)
    {
        SensorListe[i].Print_Sensor();
    }
    Serial.printf("Anzahl an Sensoren = %d\n-------------------\n", Sensor_Counter);
}


void Sensorlist::Read_all_Sensors(){

    for(int i=0; i<READINGQUANTITY;i++)
    {
        for(int i=1;i<=Sensor_Counter;i++)
        {
            SensorListe[i].Read_Sensor();
        }
        vTaskDelay(5);
    }   
}


uint16_t Sensorlist::Combine_Readings(){
    uint16_t Average_Value = 0;
    if(Valve_of_this_box != 0)
    {
        uint8_t Counter_for_Average = 0;

        for(int i = 1; i<=Sensor_Counter;i++) {
            if(SensorListe[i].Valve_Port == Valve_of_this_box)
            {
                Average_Value += SensorListe[i].Average_Reading();  
                Counter_for_Average++;              
            }
        } 
        Average_Value = Average_Value/Counter_for_Average;
    }

    return Average_Value;
}


String Sensorlist::JSON_to_String(uint8_t Valve_of_this_box, uint16_t U_Batt){
    this->Valve_of_this_box = Valve_of_this_box;
    StaticJsonDocument <1000> doc;
    String JSON_String = "";

    //K = Kennung
    //S = Sensor
    //  P = Port
    //  R = Reading

    doc["K"] = MacAdresseTarget;
        
        doc["S"][0]["P"] = Valve_of_this_box;
        doc["S"][0]["R"] = Combine_Readings();
    /*   
    for(int i = 1; i<=Sensor_Counter;i++) {
        doc["S"][i-1]["P"] = SensorListe[i].Valve_Port;
        doc["S"][i-1]["R"] = SensorListe[i].Average_Reading();
    }
    */
    doc["T"] = Temperature;
    doc["U"] = U_Batt;

    serializeJson(doc,JSON_String);

    return(JSON_String);
}

void Sensorlist::BT_Config_Data(){
    String MacAdresse = BT_JSON_Data["K"];
    Sensorlist::MacAdresseTarget = MacAdresse;

    for(JsonObject SensorValues : BT_JSON_Data["S"].as<JsonArray>())
    {
        uint8_t Port = SensorValues["P"];
        uint8_t Valve = SensorValues["V"];
        
        Add_Sensor(Valve,Port);
    }
    Print_Sensors();
}






void Sensorlist::Load_Settings(){

    Serial.println("Load Sensors-----------------");
    //Objekt der Preferences.h Bibiliothek welches zum Lesen/Speichern von Daten im EEPROM genutzt wird
    Preferences prefs;

    //Speicheradresse
    int Adresse_Int = 0;
    char Adresse_Char[8];

    //Convertiere die Int Adresse in ein für die Preferences.h passendes Format
    itoa(Adresse_Int,Adresse_Char,10);

    //Speichere/Lade bis zur Command "prefs.end();" alles unter folgendem Codewort im EEPROM
    prefs.begin("Sensorlist",false);

    uint8_t tmp_Sensor_Counter = prefs.getUShort(Adresse_Char,0);

    //Erhöhe die Speicheradresse. Dateigrößen werden automatisch berücksichtigt
    Adresse_Int +=1; 
    itoa(Adresse_Int,Adresse_Char,10);

    Serial.printf("Sensorcounter = %d\n", tmp_Sensor_Counter);

    for(int i = 1; i<=tmp_Sensor_Counter; i++)
    {
        Serial.printf("Speichervorgang Nr. %d\n SensorCounter = %d\n", i,tmp_Sensor_Counter);
        uint8_t Sensor_Port_tmp = prefs.getUShort(Adresse_Char,0);
        Adresse_Int +=1; 
        itoa(Adresse_Int,Adresse_Char,10);

        uint8_t Valve_Port_tmp = prefs.getUShort(Adresse_Char,0);
        Adresse_Int +=1; 
        itoa(Adresse_Int,Adresse_Char,10);

        Serial.print("Valve Port = ");
        Serial.println(Valve_Port_tmp);
        Serial.print("Sensor Port = ");
        Serial.println(Sensor_Port_tmp);

        Add_Sensor(Valve_Port_tmp,Sensor_Port_tmp);
    }
    MacAdresseTarget = prefs.getString(Adresse_Char,"NO_MACADRESS");
    Serial.println("MAC Adresse Target = " + MacAdresseTarget);
    
    Adresse_Int +=1; 
    itoa(Adresse_Int,Adresse_Char,10);
    
    Serial.printf("Stringlänge = %d\n", Adresse_Int);
    
    prefs.end();
}


void Sensorlist::Save_Settings(){

    Serial.println("Save Sensors---------------------");
    //Objekt der Preferences.h Bibiliothek welches zum Lesen/Speichern von Daten im EEPROM genutzt wird
    Preferences prefs;

    //Speicheradresse
    int Adresse_Int = 0;
    char Adresse_Char[8];

    //Convertiere die Int Adresse in ein für die Preferences.h passendes Format
    itoa(Adresse_Int,Adresse_Char,10);

    //Speichere/Lade bis zur Command "prefs.end();" alles unter folgendem Codewort im EEPROM
    prefs.begin("Sensorlist",false);

    Serial.printf("Sensor Counter = %d\n", Sensor_Counter);

    if(!(prefs.getUShort(Adresse_Char,0)==Sensor_Counter))
        prefs.putUShort(Adresse_Char,Sensor_Counter);

    //Erhöhe die Speicheradresse. Dateigrößen werden automatisch berücksichtigt
    Adresse_Int +=1; 
    itoa(Adresse_Int,Adresse_Char,10);
    
    Serial.printf("Adresse_int = %d\n", Adresse_Int);

    //Alle Pflanzen der Sensorenliste werden abgepeichert.
    for(uint8_t i = 1; i<=Sensor_Counter; i++)
    {
        Serial.printf("Sensor Port = %d\n", SensorListe[i].Sensor_Port);
        if(!(prefs.getUShort(Adresse_Char,0) == SensorListe[i].Sensor_Port))
        {
            //Durch putString wird der Speicherbedarf im EEPROM festgelegt, sowie der Adressraum
            prefs.putUShort(Adresse_Char,SensorListe[i].Sensor_Port);
        }  
        Adresse_Int +=1; 
        itoa(Adresse_Int,Adresse_Char,10);

        Serial.printf("Valve Port = %d\n", SensorListe[i].Valve_Port);
        if(!(prefs.getUShort(Adresse_Char,0) == SensorListe[i].Valve_Port))
        {
            //Durch putString wird der Speicherbedarf im EEPROM festgelegt, sowie der Adressraum
            prefs.putUShort(Adresse_Char,SensorListe[i].Valve_Port);
        }  
        Adresse_Int +=1; 
        itoa(Adresse_Int,Adresse_Char,10);
        
    }

    if(!(prefs.getString(Adresse_Char,"NO_MACADRESS") == MacAdresseTarget))
    {
        //Durch putString wird der Speicherbedarf im EEPROM festgelegt, sowie der Adressraum
        prefs.putString(Adresse_Char, MacAdresseTarget);
    }  
    Adresse_Int +=1; 
    itoa(Adresse_Int,Adresse_Char,10);

    Serial.printf("Stringlänge = %d\n", Adresse_Int);
    prefs.end();
}


void Sensorlist::Convert_JSON_TO_DATA_ESPNOW(){

    Serial.printf("\nConvert to JSON from ESPNOW---------------\n\n");
 
    String MacAdresse = ESPNOW_JSON_Data["MASTER"];
    String RangeTest = ESPNOW_JSON_Data["RangeTest"];

    if(MacAdresse.length() >8)
    {
        MacAdresseTarget = MacAdresse;
        Serial.println("MACAdresse Recieved = " + MacAdresseTarget);
    }

    Serial.print("Empfangene Daten =");
    Serial.println(RangeTest);
    Serial.print("Gespeicherte MAC Adresse =");
    Serial.println(MacAdresseTarget);

    if(RangeTest.length() >8 && RangeTest == MacAdresseTarget)
    //&& MacAdresseTarget == RangeTest)
    {
        Serial.println("RangeTest = " + RangeTest);
        RangeTest_status = true;
    }
}

