
// Receiver "Slave"
// The receiver listens in esp now mode and when a message is received,
// it will switch to wifi mode to transmit the data to ThingSpeak through the wifi
// router. Note that the receiver has wall power and does not need to conserve power.
// ESP Now gateway idea taken from:
// https://github.com/HarringayMakerSpace/ESP-Now/blob/master/EspNowWatsonRestartingGateway/EspNowWatsonRestartingGateway.ino

#define WIFI_CHANNEL 4  // This seems to be the quietest channel in my area

#include <ESP8266WiFi.h>
#include "password.h"
#include "ThingSpeak.h"
extern "C" {
#include <espnow.h>
#include "user_interface.h"
}

// fix mac adder so parts can be easily interchanged
void initVariant() {
  uint8_t mac[] = {0x36, 0x33, 0x33, 0x33, 0x33, 0x33};
  //uint8_t mac[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  WiFi.mode(WIFI_AP);
  bool a = wifi_set_macaddr(SOFTAP_IF, &mac[0]);
}

WiFiClient client;
volatile boolean haveReading = false;
volatile int battery_voltage;
volatile int peer_mac;

void setup()
{
  Serial.begin(115200);
  Serial.println("\r\nESP_Now running\r\n");
  pinMode( LED_BUILTIN, OUTPUT);
  digitalWrite( LED_BUILTIN, LOW);
  delay(500);
  digitalWrite( LED_BUILTIN, HIGH);

  initVariant();

  // This is the mac address of the Slave in AP Mode
  Serial.print("AP MAC: "); Serial.println(WiFi.softAPmacAddress());
  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  if (esp_now_set_self_role(ESP_NOW_ROLE_SLAVE) == 0) {
    Serial.println("Now I am slave");
  } else {
    Serial.println("Error setting role");
  }

  esp_now_register_recv_cb([](uint8_t *mac, uint8_t *data, uint8_t len)
  {
    haveReading = true;
    // adc data is 10 bit little endian
    battery_voltage = (data[0] + (((data[1] & 3) << 8)));
    peer_mac = mac[0];
  });
}

void loop()
{
  if (haveReading) {
    haveReading = false;
    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    Serial.print("Connecting to "); Serial.print(ssid);
    WiFi.begin(ssid, password);
    int i = 0;
    while (WiFi.status() != WL_CONNECTED) {
      i++;
      digitalWrite(LED_BUILTIN, LOW);
      delay(20);
      digitalWrite(LED_BUILTIN, HIGH);
      delay(450);
      Serial.print(".");
      yield();
      if (i > 20) {
        ESP.restart();
      }
    }
    tone(15, 200 + (peer_mac * 10), 200 + (peer_mac * 5));  // generate a unit unique beep on pin gpio 15
    Serial.print("\nWiFi connected, IP address: "); Serial.println(WiFi.localIP());
    // ping ThingSpeak channel and send battery voltage
    ThingSpeak.begin(client);
    ThingSpeak.setField(1, peer_mac); // send up lsb of the mac of the xmiter that signaled
    ThingSpeak.setField(2, ((float)battery_voltage) / 300);
    Serial.print("Battery voltage: "); Serial.println(((float)battery_voltage) / 300);
    ThingSpeak.writeFields( MYCHANNEL, APIKey);

    ESP.restart(); // re-enable ESP-NOW
  }
  delay(100);
}
