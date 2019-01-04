// Transmitter "Master"
// Note that this code should be optimized for low power
// as the transmitter typically runs on batteries.

#include <ESP8266WiFi.h>
extern "C" {
  #include <espnow.h>
}

#define WIFI_CHANNEL 4 // The quietest channel I could find

//MAC ADDRESS OF THE DEVICE YOU ARE SENDING TO
byte remoteMac[] = {0x5e, 0xcf, 0x7f, 0x06, 0x72, 0x6a};
const byte dataLength=7;
byte rcvData[dataLength];

//void system_deep_sleep_instant(uint32 time_in_us);

void setup()
{
  //Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin();
  WiFi.disconnect();
  //delay(2);
  //system_phy_set_max_tpw(20);    //the maximum value of RF Tx Power, unit : 0.25dBm, range [0, 82] 19dBm = 76, 10dBm = 40, 14 dBm = 56
  //delay(2);
  //Serial.print("\r\n\r\nEsp Now Transmitter with Device MAC: ");
  //Serial.println(WiFi.macAddress());

  esp_now_init();
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_add_peer(remoteMac, ESP_NOW_ROLE_SLAVE, WIFI_CHANNEL, NULL, 0);
  esp_now_send(remoteMac, rcvData, dataLength);
  //system_deep_sleep_instant(0);
  
  //ESP.deepSleepInstant(100000000, RF_NO_CAL);
  ESP.deepSleep(0);  // shut down, save power
  //delay(100); // Need this to make sleep work???? 
}  

void loop()
{
 
}

