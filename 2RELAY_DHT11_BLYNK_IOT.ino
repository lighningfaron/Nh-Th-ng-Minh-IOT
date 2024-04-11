#define BLYNK_TEMPLATE_ID "TMPLO-IqxOE2"
#define BLYNK_TEMPLATE_NAME "DHT112RELAY"
#define BLYNK_AUTH_TOKEN "eIekBngDjTkWaN3ap3ZhwQTOEugxwMPz"

#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

#include "DHTesp.h"
DHTesp dht;
#define dhtPin D0
// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "Minh Nhat";
char pass[] = "24122002";
#define button1 D1
#define button2 D2
#define relay1 D5
#define relay2 D6
unsigned long timeBlink=millis();
unsigned long timeDelay=millis();
boolean updateRelay=0;
WidgetLED LEDCONNECT(V0);
#define RELAY1 V1
#define RELAY2 V2
#define TEMP   V3
#define HUMI   V4

// Every time we connect to the cloud...
BLYNK_CONNECTED() {
  // Request the latest state from the server
  Blynk.syncVirtual(RELAY1,RELAY2);
}

// When App button is pushed - switch the state
BLYNK_WRITE(RELAY1) {
  int p = param.asInt();
  digitalWrite(relay1, p);
}
BLYNK_WRITE(RELAY2) {
  int p = param.asInt();
  digitalWrite(relay2, p);
}


ICACHE_RAM_ATTR void handleButton(){
  if(millis()-timeDelay>200){
    if(digitalRead(button1)==LOW){
      digitalWrite(relay1,!digitalRead(relay1));
    }
    if(digitalRead(button2)==LOW){
      digitalWrite(relay2,!digitalRead(relay2));
    }
    updateRelay=1;
    timeDelay=millis();
  }
}

void setup(){
  // Debug console
  Serial.begin(115200);
  pinMode(button1,INPUT_PULLUP);
  pinMode(button2,INPUT_PULLUP);
  pinMode(relay1,OUTPUT);
  pinMode(relay2,OUTPUT);
  attachInterrupt(button1,handleButton,FALLING);
  attachInterrupt(button2,handleButton,FALLING);

  dht.setup(dhtPin, DHTesp::DHT11);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
}

void loop(){
  Blynk.run();
  if(millis()-timeBlink>1000){
    if(LEDCONNECT.getValue()){
      LEDCONNECT.off();
    }else{
      LEDCONNECT.on();
    }

    float humidity = dht.getHumidity();
    float temperature = dht.getTemperature();
    Serial.print(dht.getStatusString());
    Serial.print("\t");
    Serial.print(humidity, 1);
    Serial.print("\t\t");
    Serial.println(temperature, 1);
    if(dht.getStatusString()=="OK"){
      Blynk.virtualWrite(TEMP,temperature);
      Blynk.virtualWrite(HUMI,humidity);
    }
    timeBlink=millis();
  }
  if(updateRelay==1){
    Blynk.virtualWrite(RELAY1,digitalRead(relay1));
    Blynk.virtualWrite(RELAY2,digitalRead(relay2));
    updateRelay=0;
  }
}