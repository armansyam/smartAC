#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <IRsend.h>
#include <IRremoteESP8266.h>
#include <ir_Daikin.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>   

#ifndef UNIT_TEST
#endif

#define SUHURUANG A0
#define IR_LED 4 //esp recomended D2
IRDaikinESP daikinir(IR_LED);


/************************* Adafruit.io set akun *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "username"
#define AIO_KEY         "your key"
/************ Global State (you don't need to change this!) ******************/

// membuat terhubung ke internett secara statis.
WiFiClient client;

//setup mqtt client setela wifi terkoneksi
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/

//setup sucribe & publish broker
Adafruit_MQTT_Publish suhuruang = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/suhuruang"); //Publish feed

// Setup a feed called 'onoff' for subscribing to changes.
Adafruit_MQTT_Subscribe onoffbutton = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/onoff"); //subscribe feed

Adafruit_MQTT_Subscribe suhu = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/suhu"); //subscribe feed

Adafruit_MQTT_Subscribe modeac = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/modeac"); //subscribe feed

void MQTT_connect();

void setup() {

  Serial.begin(115200);
  delay(10);
  daikinir.begin();
  Serial.println(F("Connenct to"));
//proses masuk ke APnetwork wemos
  WiFiManager wifiManager;
  Serial.println();
  wifiManager.autoConnect("SmartAC");
 Serial.println("WiFi connected");
 Serial.println("IP address: "); Serial.println(WiFi.localIP());

  // Setup MQTT subscription for onoff feed.
  mqtt.subscribe(&onoffbutton);
  mqtt.subscribe(&suhu);
  mqtt.subscribe(&modeac);
}

uint32_t x=0;

void loop() {
  MQTT_connect();


  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000))) {
    if (subscription == &onoffbutton) {
      Serial.print(F("AC "));
      Serial.println((char *)onoffbutton.lastread);
      
      if (strcmp((char *)onoffbutton.lastread, "ON") ==0) {
          daikinir.on();
          daikinir.setFan(1);
          daikinir.setMode(DAIKIN_COOL);
          daikinir.setTemp(25);
          daikinir.setSwingVertical(false);
          daikinir.setSwingHorizontal(false);
          Serial.println("AC ON");
           }
      if (strcmp((char *)onoffbutton.lastread, "OFF") ==0) {
          daikinir.off();

          Serial.println("AC OFF");
        }

    }
    if (subscription == &suhu) {
      String value = (char *) suhu.lastread;
      Serial.print(F("SUHU: "));
      Serial.println((char *)suhu.lastread);
      daikinir.setTemp(value.toFloat());
      
  
      }
    if (subscription == &modeac) {
      Serial.print(F("MODE: "));
      Serial.println((char *)modeac.lastread);
      if (strcmp((char *)modeac.lastread, "0") ==0) {
          daikinir.setMode(DAIKIN_FAN);
           }
      
      if (strcmp((char *)modeac.lastread, "1") ==0) {
          daikinir.setMode(DAIKIN_COOL);
           }
      if (strcmp((char *)modeac.lastread, "2") ==0) {
          daikinir.setMode(DAIKIN_HEAT);
           }
      if (strcmp((char *)modeac.lastread, "3") ==0) {
          daikinir.setMode(DAIKIN_HEAT);
           }
    }
         #if SEND_DAIKIN
   daikinir.send();
   #endif  // SEND_DAIKIN
  }
  

  // area publish
  int LDR = analogRead(SUHURUANG);
  
  Serial.print(F("\nSending suhuruang val "));
  Serial.print(LDR);
  Serial.print("...");
  if (! suhuruang.publish(LDR)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }

}

void MQTT_connect() {
  int8_t ret;

  if (mqtt.connected()) {
    return;
  }

  Serial.print("Menghubungkan ke MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) {
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Mengubungkan kembali ke MQTT dalam 5detik...");
       mqtt.disconnect();
       delay(5000);
       retries--;
       if (retries == 0) {

         while (1);
       }
  }
  
  
  Serial.println("MQTT Terubung!");
}
