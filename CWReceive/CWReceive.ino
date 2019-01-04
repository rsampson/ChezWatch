
// Receiver "Slave"
// The receiver listens in esp now mode and when a message is received,
// it will switch to wifi mode to transmit the data to ThingSpeak through the wifi 
// router. Note that the receiver has wall power and does not need to conserve power.
// ESP Now gateway idea taken from:
// https://github.com/HarringayMakerSpace/ESP-Now/blob/master/EspNowWatsonRestartingGateway/EspNowWatsonRestartingGateway.ino

#define WIFI_CHANNEL 4  // This seems to be the quietest channel in my area
//#define BLUELED 1 // do not use TX but drive blue LED instead
#define BLUELED D0 // NodeMCU version

#include <ESP8266WiFi.h>
#include "password.h"
#include "ThingSpeak.h"
extern "C" {
  #include <espnow.h>
  #include "user_interface.h"
}

// not used yet RAS
void initVariant() {
  uint8_t mac[] = {0x36, 0x33, 0x33, 0x33, 0x33, 0x33};
  wifi_set_macaddr(STATION_IF, mac);
}

WiFiClient client;
ADC_MODE(ADC_VCC);  // set to measure internal vcc
volatile boolean haveReading = false;

void setup()
{
  Serial.begin(115200);
  Serial.println("\r\nESP_Now MASTER CONTROLLER\r\n");
  pinMode(BLUELED, OUTPUT);
  WiFi.mode(WIFI_AP);
  //initVariant();
   // This is the mac address of the Slave in AP Mode
  Serial.print("AP MAC: "); Serial.println(WiFi.softAPmacAddress());
  WiFi.disconnect();
 
  esp_now_init();
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
    
  esp_now_register_recv_cb([](uint8_t *mac, uint8_t *data, uint8_t len)
  {
      Serial.printf("Got Something length =\t%i\n", len);
      haveReading = true;
  
    // only needed if you want to talk back  
//      if (!esp_now_is_peer_exist(macaddr))
//      {
//        Serial.print("adding peer ");
//        esp_now_add_peer(mac, ESP_NOW_ROLE_COMBO, WIFI_CHANNEL, NULL, 0);
//      }
  });
}

void loop()
{
  if (haveReading) {
      haveReading = false;
      WiFi.mode(WIFI_STA);
      Serial.print("Connecting to "); Serial.print(ssid);
      WiFi.begin(ssid, password);
      int i = 0;
      while (WiFi.status() != WL_CONNECTED) {
         i++;
         digitalWrite(BLUELED, LOW);
         delay(20);
         digitalWrite(BLUELED, HIGH);
         delay(450);
         Serial.print(".");
         yield();
         if (i > 20) ESP.restart();
      }
      Serial.print("\nWiFi connected, IP address: "); Serial.println(WiFi.localIP());
      // ping ThingSpeak channel and send battery voltage
      ThingSpeak.begin(client);
      ThingSpeak.setField(1, HIGH);
      ThingSpeak.setField(2, (float)ESP.getVcc() / 833);
      ThingSpeak.writeFields( MYCHANNEL, APIKey);
      delay(200);
      ESP.restart(); // re-enable ESP-NOW     
  }
}

