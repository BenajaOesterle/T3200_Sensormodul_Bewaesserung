#ifndef Header_h
#define Header_h

#include <WiFi.h>
#include <Arduino.h>
#include <BluetoothSerial.h>
#include <string.h>
#include <string>
#include <ArduinoJson.h>
#include <esp_now.h>



// Define LED and pushbutton pins
#define GND_OUTPUT1 12
#define GND_OUTPUT2 14
#define GND_OUTPUT3 27
#define GND_OUTPUT4 13
#define GND_OUTPUT5 18
#define VCC_OUTPUT1 26
#define VCC_OUTPUT2 25
#define VCC_OUTPUT3 33
#define VCC_OUTPUT4 15
#define ANALOG_READ1 32
#define ANALOG_READ2 35
#define ANALOG_READ3 34
#define U_BATT 39
#define TEMPERATURE 36
//#define STATUS_BUTTON 34
#define LED_INDICATOR_BUILTIN 22
#define LED_INDICATOR 23

#define BT_ONTIME_FIRST_BOOT 30 //in Seconds
#define RANGETEST_LED_CYCLE 100 //in Milliseconds
#define MAX_INT_VALUE 2147483647
#define LOW_BATT_VALUE 2000
#define VALVE_PORT_OF_THIS_BOX 1
#define NUMBER_BLINKS_LOW_BATT 3

void init_wifi();

//Timer_fnc.cpp-------------------------------------------------------------
extern bool milli_flag;
extern bool Second_flag;
extern bool LED_STATUS;

//void IRAM_ATTR onTimer();
void Timersetup();
bool OneSecEdge();
bool OneMilliSecEdge();
bool OneSecCounter();


//Deep_Sleep----------------------------------------------------------------
void DeepSleep_init_time(int Secondstime);
void Going_to_Sleep();
void print_wakeup_reason();
void hibernate();
void Check_U_Batt(int Seconds_since_on);

//Broadcast-----------------------------------------------------------------
void broadcast(const String &message);
String Initial_ESP_NOW();


//Temperature---------------------------------------------------------------
float getTemperature();

//JSON_to_String------------------------------------------------------------
String JSON_to_String(int WakeupCounter);


//Bluetooth-----------------------------------------------------------------
void Tasksetup();




#endif 