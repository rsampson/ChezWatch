
#include <ESP8266WiFi.h>
#include "ThingSpeak.h"
//const char* ssid = "*****";
//const char* password = "*****";
//const char* APIKey = "*****";
//#define MYCHANNEL *****  // my Thingspeak channel
#include "password.h"  // ssid, password, Thingspeak api key and channel defined in here


// hardware parameters
//#define SIGPIN 2 // what pins we're connected to
//#define SIGPIN D2 // NodeMCU version
//#define OUTPIN 0 // not used
#define BLUELED 1 // do not use TX but drive blue LED instead
//#define BLUELED D0 // NodeMCU version

WiFiClient client;
ADC_MODE(ADC_VCC);  // set to measure internal vcc

void setup() {

  //Serial.begin(115200);
  //pinMode(SIGPIN,OUTPUT);
  //pinMode(OUTPIN, OUTPUT);
  pinMode(BLUELED, OUTPUT);
  delay(100);
  //digitalWrite(SIGPIN, LOW);   // apply self power
  //Serial.println();
  //Serial.println();
  //Serial.print("Connecting to ");
  //Serial.println(ssid);
  //Serial.print("Flash Chip Size: ");
  //Serial.println(ESP.getFlashChipSize());

  //Serial.print("Vcc voltage: ");
  //Serial.println((float)ESP.getVcc() / 833); // 833 counts/ volt
  // ************** note! ***********************
  // this can be made lower power if the radio channel is provided
  // example:  WiFi.begin( WLAN_SSID, WLAN_PASSWD, channel, ap_mac, true );
  // See: http://bakke.online/index.php/2017/06/24/esp8266-wifi-power-reduction-avoiding-network-scan/#more-376
  WiFi.begin(ssid, password);
  delay(500);
  //WiFi.setOutputPower(16);
  WiFi.mode(WIFI_STA);
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    i++;
    digitalWrite(BLUELED, LOW);
    delay(20);
    digitalWrite(BLUELED, HIGH);
    delay(450);
    if (i > 9) ESP.restart();
  }

  ThingSpeak.begin(client);

  digitalWrite(BLUELED, LOW);
  delay(400);
  digitalWrite(BLUELED, HIGH);

  //Serial.println("");
  //Serial.println("WiFi connected");
}


void loop() {
  ThingSpeak.setField(1, HIGH);
  ThingSpeak.setField(2, (float)ESP.getVcc() / 833);
  ThingSpeak.writeFields( MYCHANNEL, APIKey);
  delay(50); // check to see if this is needed
  WiFi.disconnect();
  ESP.deepSleep(0);  // shut down
  yield();
  delay(100); // Need this to make sleep work
}
