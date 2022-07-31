// Transmitter "Master"
// When powered up, this transmitter will send battery
// voltage and peer mac address to its peer and then
// it will immediately go into low power mode.
// Note that this code should be optimized for low power
// as the transmitter typically runs on batteries.

#include <ESP8266WiFi.h>
extern "C" {
#include <espnow.h>
#include "user_interface.h"
}

#define WIFI_CHANNEL 4 // The quietest  channel I could find

//MAC ADDRESS OF THE DEVICE YOU ARE SENDING TO
uint8_t remoteMac[6] =  {0x3E, 0x71, 0xBF, 0x0D, 0xAA, 0xCE}; 
const byte dataLength = 2;

ADC_MODE(ADC_VCC);  // set to measure internal vcc
int battery_voltage;

void setup()
{
  Serial.begin(115200);  // enabling the serial port does not cause extra current drain
  Serial.print("\r\n\r\nEsp Now Transmitter with Device MAC: ");
  Serial.println(WiFi.macAddress());
  
  WiFi.mode(WIFI_STA); 
  WiFi.disconnect(); 
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    ESP.restart(); // re-enable ESP-NOW
  }

  if (esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER) == 0) {
    Serial.println("Now I am master");
  } else {
    Serial.println("Error setting role");
  }

  if (esp_now_add_peer(remoteMac, ESP_NOW_ROLE_SLAVE, WIFI_CHANNEL, NULL, 0) == 0) {
    Serial.println("Added peer");
  } else {
    Serial.println("Error adding peer");
  }

  esp_now_register_send_cb([](uint8_t* mac, uint8_t sendStatus) {
    Serial.printf("send done, status = %i\n", sendStatus);
    system_deep_sleep_instant(0);
  });
  // battery value appears to be a 12 bit value in mv
  battery_voltage = ESP.getVcc();
  Serial.printf("bat voltage = %d\n",battery_voltage);
  
  //esp_now_send(remoteMac, (u8*)&battery_voltage, dataLength);
  esp_now_send(0, (u8*)&battery_voltage, dataLength);
  delay(200); // required for send to complete
  system_deep_sleep_instant(0);
}

void loop()
{
}
