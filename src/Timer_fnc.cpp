#include <Arduino.h>
#include "Header.h"


//extern Variables------------
bool milli_flag = false;
bool Second_flag = false;
//----------------------------

//Hardware_Timer Objetpointer
hw_timer_t * timer = NULL;
uint16_t Timervalue = 0;

//Interrupt-Funktion
void IRAM_ATTR onTimer(){

  //Counter um von Millisekunden aufSekunde zu kommen ohne zusätzlichen Hardware Interupt
  if(Timervalue >= 999)
  {
    Timervalue = 0;
    Second_flag = true;
  }
  else
  {
    Timervalue++;
  }

  milli_flag = true;
}

void Timersetup(){
  //Hardwaretimer Nr1 wird mit einem Prescaler von 80 betrieben 
  //Timer läuft mit 80MHz somit 1.000.000 Ticks/Sekunde

  Serial.println("Timer initialisiert");

  timer = timerBegin(1,80,true);

  //Zuweisung der Interuptfunktion
  timerAttachInterrupt(timer, &onTimer, true);

  //Interrupt alle 1000 Ticks somit 1x pro Millisekunde
  timerAlarmWrite(timer, 1000, true);
  timerAlarmEnable(timer);
}

float getTemperature(){
  int Sensorwert = 0;
  float Temperaturevalue = 0.0;
  float Zero_Grad = 4096/3;
  float PT_Faktor = (138.51/338.51);
  float Difference = 0.0;


  for(uint8_t i=0; i<10;i++)
  {
    Sensorwert += analogRead(TEMPERATURE);
    delay(2);
  }
  Sensorwert = Sensorwert/10;
  Serial.printf("Sensorwert = %d\n", Sensorwert);
  
  Difference = (float)Sensorwert - Zero_Grad;
  Serial.printf("Difference = %d\n", Difference);

  Temperaturevalue = Difference * PT_Faktor;

  Serial.printf("Temperatur = %f\n", Temperaturevalue);

  return Temperaturevalue;
}

