
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


void setup()
{
  tone(D7, 880, 500); // beep when alarm received
  Serial.begin(115200);
  Serial.println("\r\nESP_Now running\r\n");

  pinMode( LED_BUILTIN, OUTPUT);
  digitalWrite( LED_BUILTIN, LOW);
  delay(500);
  digitalWrite( LED_BUILTIN, HIGH);

  
// fix mac adder to broadcast mode so parts can be easily interchanged
  WiFi.mode(WIFI_STA); 
  uint8_t mac[6] =  {0x3E, 0x71, 0xBF, 0x0D, 0xAA, 0xCE};   
  wifi_set_macaddr(STATION_IF, const_cast<uint8*>(mac));
  WiFi.disconnect();

  Serial.println("AP MAC: "); Serial.println(WiFi.softAPmacAddress());
  Serial.println("MAC: "); Serial.println(WiFi.macAddress());

  // Init ESP-NOW
  if (esp_now_init() == 0)  // register call back if OK
  {
    // esp_now_set_self_role(ESP_NOW_ROLE_SLAVE); need to test
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
    Serial.println("ESP-NOW Init Failed");
    ESP.restart();
  }
}

void loop()
{
  if (haveReading) {
    haveReading = false;
    Serial.println("Received alarm");
    // announce alarm source
    switch (peer_mac) {
      case 230:
        statusString = String("Movement on driveway");
        break;
      case 155:
        statusString = String("Motion sensor activated");
        break;
      case 212:
        statusString = String("Motion detected in garage");
        break;
      case 27:
        statusString = String("Front gate open");
        break;
      case 186:
        statusString = String("Back gate entry");
        break;
      case 204:
        statusString = String("Movement near front door");
        break;
      default:
        statusString = String("Unknown alarm source");
        break;
    }
    // set up voice synth
    out = new AudioOutputI2SNoDAC(); // initialize voice audio
    out->begin();
    sam->SetPitch(68);  // default was 64
    sam->SetMouth(135);  //was 128
    sam->SetThroat(135); // was 128
    sam->SetSpeed(100); // was 72
    char Buf[50];
    statusString.toCharArray(Buf, 50);
    sam->Say(out, Buf); // audio anounce of alarm

    if (battery_voltage < 2.5) {
      delay(500);
      sam->Say(out, "Warning! Warning! Low battery!");
    }
    
    esp_now_deinit();  // disconnect from esp now mode and establish WiFi mode
    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    Serial.print("Connecting to "); Serial.print(ssid);
    WiFi.begin(ssid, password);
    int i = 0;
    while (WiFi.status() != WL_CONNECTED) {

      digitalWrite(LED_BUILTIN, LOW);
      delay(20);
      digitalWrite(LED_BUILTIN, HIGH);
      delay(250);
      Serial.print(".");
      if (i++ > 20) {
        WiFi.printDiag(Serial);
        ESP.restart();
      }
    }
    Serial.print("\nWiFi connected, IP address: "); Serial.println(WiFi.localIP());

    // contact ThingSpeak channel and send battery voltage and status
    ThingSpeak.begin(client);
    ThingSpeak.setField(1, peer_mac); // send up lsb of the mac of the xmiter that signaled
    ThingSpeak.setField(2, battery_voltage);
    ThingSpeak.setStatus(String(peer_mac) + String("  ") + statusString +
                         String(" Battery Volts= ") + String(battery_voltage));
    ThingSpeak.writeFields( MYCHANNEL, APIKey);  
                         
    Serial.print("Battery voltage: "); Serial.println(((float)battery_voltage) / 1000);
    Serial.print("Peer mac: "); Serial.println(peer_mac);

    ESP.restart(); // restart into ESP-NOW mode
  }
  delay(100);
}
