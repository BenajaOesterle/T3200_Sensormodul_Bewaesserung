#include <Arduino.h>
#include "Header.h"
#include "SensorLib.h"

// Include Libraries
#include <WiFi.h>
#include <esp_now.h>
//#include <WiFiUdp.h>
#include <esp_wifi.h>

void formatMacAddress(const uint8_t *macAddr, char *buffer, int maxLength)
// Formats MAC Address
{
  snprintf(buffer, maxLength, "%02x:%02x:%02x:%02x:%02x:%02x", macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);
}

void receiveCallback(const uint8_t *macAddr, const uint8_t *data, int dataLen)
// Called when data is received
{
  // Only allow a maximum of 250 characters in the message + a null terminating byte
  char buffer[ESP_NOW_MAX_DATA_LEN + 1];
  int msgLen = min(ESP_NOW_MAX_DATA_LEN, dataLen);
  strncpy(buffer, (const char *)data, msgLen);

  // Make sure we are null terminated
  buffer[msgLen] = 0;

  // Format the MAC address
  char macStr[18];
  formatMacAddress(macAddr, macStr, 18);

  // Send Debug log message to the serial port
  Serial.printf("Received message from: %s\n", macStr);

  Serial.println("Buffer = " + String(buffer));

  // Check switch status
  if (strcmp("on", buffer) == 0)
  {
    //ledOn = true;
    //analogWrite(STATUS_LED,10);
  }
  //Deserialisiere die Datten aus dem Buffer und schreibe diese in die Sensorliste
  deserializeJson(Sensorlist::ESPNOW_JSON_Data,buffer);

  //Gebe die Daten geordnet im Seriellen Monitor aus
  serializeJsonPretty(Sensorlist::ESPNOW_JSON_Data,Serial);
  Sensorlist::NEW_ESPNOW_Data = true;
}


void sentCallback(const uint8_t *macAddr, esp_now_send_status_t status)
// Called when data is sent
{
  char macStr[18];
  formatMacAddress(macAddr, macStr, 18);
  Serial.print("Last Packet Sent to: ");
  Serial.println(macStr);
  Serial.print("Last Packet Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void broadcast(const String &message)
{
  // Broadcast eine Nachricht an jedes Gerät mit ESP-NOW im Umkreis
  uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

  //Übergebe die Broadcast Adresse via einer Hilfsvariable peerInfo
  esp_now_peer_info_t peerInfo = {};
  memcpy(&peerInfo.peer_addr, broadcastAddress, 6);
  if (!esp_now_is_peer_exist(broadcastAddress))
  {
    esp_now_add_peer(&peerInfo);
  }
  // Broadcaste übergebene Nachricht
  esp_err_t result = esp_now_send(broadcastAddress, (const uint8_t *)message.c_str(), message.length());
    // Print results to serial monitor
  if (result == ESP_OK)
  {
    Serial.println("Broadcast message success");
  }
  else if (result == ESP_ERR_ESPNOW_NOT_INIT)
  {
    Serial.println("ESP-NOW not Init.");
  }
  else if (result == ESP_ERR_ESPNOW_ARG)
  {
    Serial.println("Invalid Argument");
  }
  else if (result == ESP_ERR_ESPNOW_INTERNAL)
  {
    Serial.println("Internal Error");
  }
  else if (result == ESP_ERR_ESPNOW_NO_MEM)
  {
    Serial.println("ESP_ERR_ESPNOW_NO_MEM");
  }
  else if (result == ESP_ERR_ESPNOW_NOT_FOUND)
  {
    Serial.println("Peer not found.");
  }
  else
  {
    Serial.println("Unknown error");
  }
}

String Initial_ESP_NOW()
{
  //Verwende WiFi im STA Modus also ohne Access-Point
  WiFi.mode(WIFI_STA);
  Serial.println("ESP-NOW Broadcast Demo");

  //Benutze das WiFi Long Range Protokoll um die Reichweite zu erhöhen
  int a= esp_wifi_set_protocol( WIFI_IF_STA, WIFI_PROTOCOL_LR );
  Serial.println(a);

  // Print MAC address
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());
  String MacAdresse = WiFi.macAddress();

  // Disconnect from WiFi
  WiFi.disconnect();
  
    // Initialize ESP-NOW
  if (esp_now_init() == ESP_OK)
  {
    Serial.println("ESP-NOW Init Success");
    //Definiere folgende Funktionen als "Sende-/Empfangsfunktion"
    esp_now_register_recv_cb(receiveCallback);
    esp_now_register_send_cb(sentCallback);
  }
  else
  {
    //Im Fehlerfall starte den ESP neu
    Serial.println("ESP-NOW Init Failed");
    delay(3000);
    ESP.restart();
  }
  return(MacAdresse);
}