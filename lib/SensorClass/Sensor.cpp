#include <Arduino.h>
#include <string.h>
#include <string>
#include <ArduinoJson.h>

#include "SensorLib.h"



void Sensor::Add_new_Value(uint16_t reading){

    Sensor_readinglist[curr_readinglist] = reading;
    if(curr_readinglist<(READINGQUANTITY - 1))
        curr_readinglist++;
    else
        curr_readinglist = 0;
}

void Sensor::Init_readinglist(){
    for(int i = 0; i<READINGQUANTITY; i++)
    {
       Add_new_Value(0); 
    }
    curr_readinglist = 0;
}

void Sensor::Print_Sensor(){
    Serial.printf("Sensor Port = %d\nValve Port = %d\nReadings:\n", Sensor_Port, Valve_Port);
    for(int i = 0; i<READINGQUANTITY; i++)
    {
        Serial.printf("  Nr%d = %d\n",i,Sensor_readinglist[i]);
    }
    Serial.println("----------------");
}

void Sensor::Read_Sensor(){
    uint32_t Sensorvalue = 0;

    for(int i=0; i<READINGS_PER_MESSUREMENT;i++)
    {
        Sensorvalue += analogRead(Sensor_Port);
    }
        

    Sensorvalue = Sensorvalue/READINGS_PER_MESSUREMENT;

    Add_new_Value(Sensorvalue);
}


uint16_t Sensor::Average_Reading(){
    uint32_t Average = 0;
    for(int i=0; i<READINGQUANTITY;i++)
    {
        Average += Sensor_readinglist[i];
    }
    Average = Average/READINGQUANTITY;
    return(Average);
}