// Transmitter "Master"
// Note that this code should be optimized for low power
// as the transmitter typically runs on batteries.

#include <ESP8266WiFi.h>
extern "C" {
  #include <espnow.h>
  #include "user_interface.h"
}

#define WIFI_CHANNEL 4 // The quietest channel I could find

//MAC ADDRESS OF THE DEVICE YOU ARE SENDING TO
byte remoteMac[] = {0x36, 0x33, 0x33, 0x33, 0x33, 0x33};
const byte dataLength=2;
ADC_MODE(ADC_VCC);  // set to measure internal vcc
int battery_voltage;

void setup()
{
  Serial.begin(115200);  // enabling the serial port does not cause extra current drain
  WiFi.mode(WIFI_STA);
  WiFi.begin();
  WiFi.disconnect();
  //system_phy_set_max_tpw(20);    //the maximum value of RF Tx Power, unit : 0.25dBm, range [0, 82] 19dBm = 76, 10dBm = 40, 14 dBm = 56
  Serial.print("\r\n\r\nEsp Now Transmitter with Device MAC: ");
  Serial.println(WiFi.macAddress());

  esp_now_init();
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_add_peer(remoteMac, ESP_NOW_ROLE_SLAVE, WIFI_CHANNEL, NULL, 0);
  battery_voltage = ESP.getVcc();
  battery_voltage = ESP.getVcc();  // may need to read twice to be reliable???
  esp_now_send(remoteMac, (u8*)&battery_voltage, dataLength); // better to send local mac!
  delay(10); // required for send to complete
  system_deep_sleep_instant(0);

  //ESP.deepSleep(0);  // It is not known if these last two lines are needed!!
  //delay(100);
}  

void loop()
{
  ESP.deepSleep(0);  // It is not known if these last two lines are needed!!
}

