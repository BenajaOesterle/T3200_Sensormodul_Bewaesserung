#include <Arduino.h>
#include "Header.h"

#define uS_TO_S_FACTOR 1000000ULL  /* Conversion factor for micro seconds to seconds */



void print_wakeup_reason(){
  //Gebe den Grund des Erwachens auf dem Seriellen Monitor aus
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();
  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}

void DeepSleep_init_time(int Secondstime)
{
  //Stelle die Zeit in Sekunden ein, nach welcher der ESP32 wieder erwachen soll.
  esp_sleep_enable_timer_wakeup(Secondstime * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(Secondstime) + " Seconds");
}

void Going_to_Sleep()
{
    Serial.println("Going to sleep now");
    //Warte bis Serielle Daten fertig übertragen sind und gehen dann erst in den Deep Sleep
    Serial.flush(); 
    esp_deep_sleep_start();
}

void hibernate()
{  
    //Schalte zusätzlich zum Deep Sleep auch die anderern RTC Funktionen ab um maximal Akku zu sparen.
    DeepSleep_init_time(MAX_INT_VALUE);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH,   ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_XTAL,         ESP_PD_OPTION_OFF);

    Going_to_Sleep();
}

void Check_U_Batt(int Seconds_since_on)
{
  //Wenn der Ladezustand der Batterie zu niedrig ist
  if(analogRead(U_BATT)< LOW_BATT_VALUE)
  {
    Serial.printf("Low Value = %d\n", analogRead(U_BATT));

    //Bei einem frischen Start des ESP32 3x schnell Blinekn
    if(Seconds_since_on<1)
    {
      for(uint8_t i = 0; i<(NUMBER_BLINKS_LOW_BATT*2);i++)
      {
        if(i%2)
        {
          digitalWrite(LED_INDICATOR, LOW);
        }
        else
        {
          digitalWrite(LED_INDICATOR, HIGH);
        }
        vTaskDelay(200);
      }
    }
    hibernate();
  }
  else
  {
    Serial.printf("High Value = %d\n", analogRead(U_BATT));
    
    digitalWrite(LED_INDICATOR, HIGH);
    //Bei einem frischen Start des ESP32 eine Sekunde lang leuchten
    if(Seconds_since_on<1)
      vTaskDelay(1000);

    digitalWrite(LED_INDICATOR, LOW);
  } 
}