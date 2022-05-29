
// Receiver, routes received esp now signal to wifi.
// The receiver listens in esp now mode and when a message is received,
// it will switch to wifi mode to transmit the data to ThingSpeak through the wifi
// router. Note that the receiver has wall power and does not need to conserve power.
// ESP Now gateway idea taken from:
// https://github.com/HarringayMakerSpace/ESP-Now/blob/master/EspNowWatsonRestartingGateway/EspNowWatsonRestartingGateway.ino
// Uses the excellent voice synthesizer found at: https://github.com/earlephilhower/ESP8266SAM/tree/master/src

#include <ESP8266WiFi.h>
#include "password.h"   // keep your ssid, password, ThingSpeak channel and key in here in this format:

//const char* ssid = "xxxx";
//const char* password = "xxxx";
//const char* APIKey = "xxxx";
//#define MYCHANNEL 1234

#include "ThingSpeak.h"
#include <Arduino.h>
#include <ESP8266SAM.h>
#include "AudioOutputI2SNoDAC.h"

extern "C" {
#include <espnow.h>
#include "user_interface.h"
}


AudioOutputI2SNoDAC *out = NULL;  //voice resources
ESP8266SAM *sam = new ESP8266SAM;
WiFiClient client;
volatile boolean haveReading = false;
float battery_voltage;
volatile int peer_mac;
String statusString = "";


// fix mac adder to broadcast mode so parts can be easily interchanged

void initMac() {
  WiFi.mode(WIFI_STA); // was AP
  //uint8_t mac[] = {0x36, 0x33, 0x33, 0x33, 0x33, 0x33};
  uint8_t mac[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // this puts it in broadcast mode
  bool a = wifi_set_macaddr(STATION_IF, &mac[0]);
}

void setup()
{
  //Serial.begin(115200);
  //Serial.println("\r\nESP_Now running\r\n");

  pinMode( LED_BUILTIN, OUTPUT);
  digitalWrite( LED_BUILTIN, LOW);
  delay(500);
  digitalWrite( LED_BUILTIN, HIGH);
  
  out = new AudioOutputI2SNoDAC(); // initialize voice audio
  out->begin();
  //sam->Say(out,"hello");
  
  initMac();  // required only if not receiving broadcasts  
  // This is the mac address of the Slave in AP Mode
  //Serial.print("AP MAC: "); Serial.println(WiFi.softAPmacAddress());
  
  // Init ESP-NOW
  if (esp_now_init() == 0)  // register call back if OK
  {
      esp_now_register_recv_cb([](uint8_t *mac, uint8_t *data, uint8_t len)
      {
        haveReading = true;
        // adc data is 12 bit little endian
        battery_voltage = (float)(data[0] + (((data[1] & 15) << 8)));
        battery_voltage = battery_voltage / 1000;
        peer_mac = mac[5];  // enough mac to identify sensor
      });
  }
  else
  {
    //Serial.println("ESP-NOW Init Failed");
    ESP.restart();
  }
}

void loop()
{
  if (haveReading) {
    haveReading = false;
    WiFi.disconnect(); // disconnect from esp now mode
    WiFi.mode(WIFI_STA);
    //Serial.print("Connecting to "); Serial.print(ssid);
    WiFi.begin(ssid, password);
    int i = 0;
    while (WiFi.status() != WL_CONNECTED) {
      i++;
      digitalWrite(LED_BUILTIN, LOW);
      delay(20);
      digitalWrite(LED_BUILTIN, HIGH);
      delay(450);
      //Serial.print(".");
      if (i > 20) {
        ESP.restart();
      }
    }
    //Serial.print("\nWiFi connected, IP address: "); Serial.println(WiFi.localIP());

    // contact ThingSpeak channel and send battery voltage
     ThingSpeak.begin(client);
     ThingSpeak.setField(1, peer_mac); // send up lsb of the mac of the xmiter that signaled
     ThingSpeak.setField(2, battery_voltage);
    
    // generating status
    switch(peer_mac) {
      case 155: 
        statusString = String("Motion sensor");
        break;
      case 212: 
        statusString = String("Garage");
        break;
      case 27: 
        statusString = String("Front gate");
        break;
      case 186: 
        statusString = String("Rear gate");
        break;
      case 204: 
        statusString = String("Front door");
        break;
      default: 
        statusString = String("Unknown");
        break;
    }
    
    sam->Say(out,"alarm");
    delay(300);
    char Buf[50];
    statusString.toCharArray(Buf, 50);
    sam->Say(out, Buf); // audio anounce of alarm
    
    if(battery_voltage < 2.5) {
      delay(400);
      sam->Say(out,"Warning! Warning! Low battery!");
    }
    
    ThingSpeak.setStatus(statusString + 
                         String(" Battery Volts= ") + String(battery_voltage)
                         );
    //Serial.print("Battery voltage: "); Serial.println(((float)battery_voltage) / 1000);
    //Serial.print("Peer mac: "); Serial.println(peer_mac);
    ThingSpeak.writeFields( MYCHANNEL, APIKey);

    ESP.restart(); // re-enable ESP-NOW
  }
  delay(100);
}
