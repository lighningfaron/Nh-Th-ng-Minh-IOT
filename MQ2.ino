#define BLYNK_TEMPLATE_ID "TMPL67akvAOGc"
#define BLYNK_TEMPLATE_NAME "MQ"

#define BLYNK_FIRMWARE_VERSION        "0.1.0"
#define BLYNK_PRINT Serial
#define APP_DEBUG
#define USE_NODE_MCU_BOARD
#include "BlynkEdgent.h"
#include <Servo.h>
Servo myservo;
BlynkTimer timer;
int timerID1,timerID2;
int mq2_value;
int mucCanhbao=500;
int buzzer=5; //D1
int ledMode = 12; //D6 led hiển thị chế độ hoạt động
int servopin=4; // D2 Kết nối servo
int button1=0; //D3 Bật tắt chế độ cảnh báo
int button2=14; // D5 Điều khiển cửa
boolean button1State=HIGH;
boolean button2State=HIGH;
boolean runMode=1;//Bật/tắt chế độ cảnh báo
boolean canhbaoState=0;
boolean cuaState=0;
WidgetLED led(V0);
#define KHIGAS V1
#define MUCCANHBAO V2
#define RUNMODE V3
#define TRANGTHAICB V4
#define SERVO V5

void setup()
{
  Serial.begin(115200);
  delay(100);
  pinMode(button1,INPUT_PULLUP);
  pinMode(button2,INPUT_PULLUP);
  pinMode(buzzer,OUTPUT);
  digitalWrite(buzzer,LOW); //Tắt buzzer
  pinMode(ledMode,OUTPUT);
  digitalWrite(ledMode,LOW);//Tắt led mode
  myservo.attach(servopin);
  BlynkEdgent.begin();
  timerID1 = timer.setInterval(1000L,handleTimerID1);
}

void loop() {
  BlynkEdgent.run();
  timer.run();
  if(digitalRead(button1)==LOW){
    if(button1State==HIGH){
      button1State=LOW;
      runMode=!runMode;
      digitalWrite(ledMode,runMode);
      Serial.println("Run mode: " + String(runMode));
      Blynk.virtualWrite(RUNMODE,runMode);
      delay(200);
    }
  }else{
    button1State=HIGH;
  }
  if(digitalRead(button2)==LOW){
    if(button2State==HIGH){
      controlDoor();
      Blynk.virtualWrite(SERVO,cuaState);
      delay(200);
    }
  }else{
    button2State=HIGH;
  }
}
void handleTimerID1(){
  mq2_value = analogRead(A0);
  Blynk.virtualWrite(KHIGAS,mq2_value);
  if(led.getValue()) {
    led.off();
  } else {
    led.on();
  }
  if(runMode==1){
    if(mq2_value>mucCanhbao){
      if(canhbaoState==0){
        canhbaoState=1;
        Blynk.logEvent("canhbao", String("Cảnh báo! Khí gas=" + String(mq2_value)+" vượt quá mức cho phép!"));
        digitalWrite(buzzer,HIGH);
        Blynk.virtualWrite(TRANGTHAICB,HIGH);
        Serial.println("Đã bật cảnh báo!");
        timerID2 = timer.setTimeout(60000L,handleTimerID2);
      }
      if(cuaState==0){
        for (int pos = 180; pos >= 0; pos -= 5) {
          myservo.write(pos);
          delay(0);
        }
        cuaState=1;
      }
      Blynk.virtualWrite(SERVO,cuaState);
    }
  }else{
    digitalWrite(buzzer,LOW);
    Blynk.virtualWrite(TRANGTHAICB,LOW);
    Serial.println("Đã tắt cảnh báo!");
    canhbaoState=0;
  }
}
void handleTimerID2(){
  canhbaoState=0;
  if(mq2_value<mucCanhbao){
    digitalWrite(buzzer,LOW);
    Blynk.virtualWrite(TRANGTHAICB,LOW);
    Serial.println("Đã tắt cảnh báo!");
  }
}
BLYNK_CONNECTED() {
  Blynk.syncVirtual(RUNMODE,MUCCANHBAO,SERVO);
}
BLYNK_WRITE(MUCCANHBAO) {
  mucCanhbao = param.asInt();
  Serial.println("Mức cảnh báo: " + String(mucCanhbao));
}
BLYNK_WRITE(RUNMODE) {
  runMode = param.asInt();
  digitalWrite(ledMode,runMode);
}
BLYNK_WRITE(SERVO){
  cuaState = !param.asInt();
  controlDoor();
}
void controlDoor(){
  if(cuaState==1){
    for (int pos = 0; pos <= 180; pos += 5){
      myservo.write(pos);
      delay(0);
    }
    cuaState=0;
  }else{
    for (int pos = 180; pos >= 0; pos -= 5) {
      myservo.write(pos);
      delay(0);
    }
    cuaState=1;
  }
}
