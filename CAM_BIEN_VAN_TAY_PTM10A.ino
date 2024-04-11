#include <SimpleTimer.h>
#include <Adafruit_Fingerprint.h>
SoftwareSerial mySerial(2, 3);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
uint8_t id=0;
const int buzzer = A0;
SimpleTimer timer;
int timerID;
unsigned long timesScan=millis();
boolean setupState=0;
const int relayPin = A1;
#include<EEPROM.h>
#include <Keypad.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,16,2); 

const int button = 4;
boolean buttonState=HIGH;

int i=0;
boolean input_state=0;
char password[4];
char initial_password[4],new_password[4];
char key_pressed=0;
const byte rows = 4; 
const byte columns = 4; 
char hexaKeys[rows][columns] = 
{
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte row_pins[rows] = {12, 11, 10, 9}; 
byte column_pins[columns] = {8,7,6,5};   
Keypad keypad_key = Keypad( makeKeymap(hexaKeys), row_pins, column_pins, rows, columns);
boolean setupFingerState=0;
#define TIME_OPEN 5000
unsigned long timeSendData=millis();

void setup()
{
  Serial.begin(9600);
  delay(100);
  pinMode(buzzer,OUTPUT);
  digitalWrite(buzzer,LOW);
  pinMode(button,INPUT_PULLUP);
  pinMode(relayPin,OUTPUT);
  digitalWrite(relayPin,LOW);

  // Serial.println("\n\nAdafruit Fingerprint sensor enrollment");
  finger.begin(57600);
  // if (finger.verifyPassword()) {
  //   Serial.println("Found fingerprint sensor!");
  // } else {
  //   Serial.println("Did not find fingerprint sensor :(");
  //   while (1) { delay(1); }
  // }
  // Serial.println(F("Reading sensor parameters"));
  finger.getParameters();
  // Serial.print(F("Status: 0x")); Serial.println(finger.status_reg, HEX);
  // Serial.print(F("Sys ID: 0x")); Serial.println(finger.system_id, HEX);
  // Serial.print(F("Capacity: ")); Serial.println(finger.capacity);
  // Serial.print(F("Security level: ")); Serial.println(finger.security_level);
  // Serial.print(F("Device address: ")); Serial.println(finger.device_addr, HEX);
  // Serial.print(F("Packet len: ")); Serial.println(finger.packet_len);
  // Serial.print(F("Baud rate: ")); Serial.println(finger.baud_rate);
  
  finger.getTemplateCount();

  if (finger.templateCount == 0) {
   // Serial.print("Sensor doesn't contain any fingerprint data. Please run the 'enroll' example.");
  }
  else {
    // Serial.println("Waiting for valid finger...");
    // Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
    id=finger.getTemplateCount()+2;
    // Serial.println("ID: "+String(id));
  }
  // Serial.println("Ready to enroll a fingerprint!");
  initialpassword();

  lcd.init();                    
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Fingerprint scan");
  lcd.setCursor(3,1);
  lcd.print("or Pres A!");
}

void loop()                     // run over and over again
{
  if(millis()-timeSendData>1000){
    Serial.println("RELAY:"+String(digitalRead(relayPin)));
    timeSendData=millis();
  }
  String buffer="";
  if(Serial.available()){
    while(Serial.available()){
      buffer = Serial.readString();
    }
    if(buffer.length()>0){
      Serial.println(buffer);
      buffer.trim();
      if(buffer=="RLON"){
        runOpen();
      }
    }
  }
  if(digitalRead(button)==LOW){
    if(buttonState==HIGH){
      //Mở cửa
      runOpen();
      buttonState=LOW;
      delay(200);
    }
  }else{
    buttonState=HIGH;
  }
  timer.run();
  if(setupState==1){
    while (!getFingerprintEnroll() );
  }else{
    if(millis()-timesScan>500){
      getFingerprintID();
      timesScan=millis();
    }
  }

  key_pressed = keypad_key.getKey();
  if(key_pressed=='*'){
    change_password();
  }
  if(key_pressed=='#'){
    input_state=1;
    i=0;
    setup_finger();
  }
  if(key_pressed=='A'){
    input_state=1;
    i=0;
    open_door();
  }
  if(setupFingerState==1){
    if(key_pressed=='B'){
      setupState=1;
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Add new finger");
      lcd.setCursor(0,1);
      lcd.print("pres D to Exit");
      delay(1000);
    }else if(key_pressed=='C'){
      finger.emptyDatabase();
      // Serial.println("Now database is empty :)");
      id=1;
      setupState=1;
      beep(200);
      beep(200);
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Data finger is");
      lcd.setCursor(4,1);
      lcd.print("deleted");
      delay(1000);
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Add new finger");
      lcd.setCursor(0,1);
      lcd.print("pres D to Exit");
      delay(1000);
    }
  }
}
void initialpassword(){
  // Serial.println();
  // Serial.print("Password: " );
  for(int j=0;j<4;j++){
    initial_password[j]=EEPROM.read(j);
    // Serial.print(char(initial_password[j]));
  }
  // Serial.println();
}
//Thay đổi password
void change_password(){
  int j=0;
  lcd.clear();
  lcd.print("Current Password");
  lcd.setCursor(0,1);
  while(j<4){
    char key=keypad_key.getKey();
    if(key){
      new_password[j++]=key;
      lcd.print(key);
      delay(500);
      lcd.setCursor(j-1,1);
      lcd.print("*");
    }
    key=0;
  }
  delay(500);

  if((strncmp(new_password, initial_password, 4))){
    lcd.clear();
    lcd.print("Wrong Password");
    lcd.setCursor(0,1);
    lcd.print("Try Again");
    beep(50);
    beep(50);
    beep(50);
    beep(50);
    delay(1000);
  }else{
    j=0;
    lcd.clear();
    lcd.print("New Password:");
    lcd.setCursor(0,1);
    while(j<4){
      char key=keypad_key.getKey();
      if(key){
        initial_password[j]=key;
        lcd.print(key);
        delay(500);
        lcd.setCursor(j,1);
        lcd.print("*");
        EEPROM.write(j,key);
        j++;
      }
    }
    lcd.print("Pass Changed");
    delay(1000);
  }
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Fingerprint scan");
  lcd.setCursor(3,1);
  lcd.print("or Pres A!");
  key_pressed=0;
}
void setup_finger(){
  lcd.clear();
  lcd.print("Enter Password");
  lcd.setCursor(0,1);
  while(input_state==1){
    char key=keypad_key.getKey();
    if(key){
      password[i++]=key;
      lcd.print(key);
      delay(500);
      lcd.setCursor(i-1,1);
      lcd.print("*");
    }
    if(i==4){
      input_state=0;
      delay(200);
      for(int j=0;j<4;j++)
        initial_password[j]=EEPROM.read(j);
      if(!(strncmp(password, initial_password,4))){
        lcd.clear();
        lcd.print("Pass Accepted");
        delay(1000);
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Pres B to Add");
        lcd.setCursor(1,1);
        lcd.print("C to Clear!"); 
        setupFingerState=1;       
      }else{
        lcd.clear();
        lcd.print("Wrong Password");
        lcd.setCursor(0,1);
        lcd.print("Try Again");
        beep(50);
        beep(50);
        beep(50);
        beep(50);
        i=0;
        delay(1000);
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Fingerprint scan");
        lcd.setCursor(3,1);
        lcd.print("or Pres A!");
      }
    }
  }
}
//Mở cửa bằng mật khẩu nhập từ phím
void open_door(){
  lcd.clear();
  lcd.print("Enter Password");
  lcd.setCursor(0,1);
  while(input_state==1){
    char key=keypad_key.getKey();
    if(key){
      password[i++]=key;
      lcd.print(key);
      delay(500);
      lcd.setCursor(i-1,1);
      lcd.print("*");
    }
    if(i==4){
      input_state=0;
      delay(200);
      for(int j=0;j<4;j++)
        initial_password[j]=EEPROM.read(j);
      if(!(strncmp(password, initial_password,4))){
        lcd.clear();
        lcd.print("Pass Accepted");
        //Mở cửa
        runOpen();
      }else{
        lcd.clear();
        lcd.print("Wrong Password");
        lcd.setCursor(0,1);
        lcd.print("Try Again");
        beep(50);
        beep(50);
        beep(50);
        beep(50);
        i=0;
      }
      delay(1000);
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Fingerprint scan");
      lcd.setCursor(3,1);
      lcd.print("or Pres A!");
    }
  }
}
void beep(int i){
  digitalWrite(buzzer,HIGH);
  delay(i);
  digitalWrite(buzzer,LOW);
  delay(i);
}
//Thêm vân tay mới vào bộ nhớ
uint8_t getFingerprintEnroll() {
  int p = -1;
  // Serial.print("Waiting for valid finger to enroll as #"); Serial.println(id);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Fingerprint scan!");
  lcd.setCursor(0,1);
  lcd.print("needs more!");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      // Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      // Serial.print(".");
      key_pressed = keypad_key.getKey();
      if(key_pressed=='D'){
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Fingerprint scan");
        lcd.setCursor(3,1);
        lcd.print("or Pres A!");
        setupState=0;
        return true;
      }
      break;
    default:
      // Serial.println("Error");
      break;
    }
  }
  // OK success!
  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      // Serial.println("Image converted");
      break;
    default:
      // Serial.println("Error");
      return p;
  }

  // Serial.println("Remove finger");
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Remove finger!");
  beep(200);
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  // Serial.print("ID "); Serial.println(id);
  p = -1;
  // Serial.println("Place same finger again");
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Place same");
  lcd.setCursor(0,1);
  lcd.print("finger again!");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      // Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      // Serial.print(".");
      key_pressed = keypad_key.getKey();
      if(key_pressed=='D'){
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Fingerprint scan");
        lcd.setCursor(3,1);
        lcd.print("or Pres A!");
        setupState=0;
        return true;
      }
      break;
    default:
      // Serial.println("Error");
      break;
    }
  }
  // OK success!
  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      // Serial.println("Image converted");
      break;
    default:
      // Serial.println("Unknown error");
      return p;
  }
  // OK converted!
  // Serial.print("Creating model for #");  Serial.println(id);
  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    // Serial.println("Prints matched!");
  }else {
    // Serial.println("Error");
    return p;
  }
  // Serial.print("ID "); Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    //Serial.println("Stored!");
  }else {
    // Serial.println("Error");
    return p;
  }
  id++;
  beep(200);
  beep(200);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Fingerprint saved!");
  delay(1000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Add new finger");
  lcd.setCursor(0,1);
  lcd.print("pres D to Exit");
  delay(1000);
  return true;
}
//Quét vân tay mở cửa
uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      // Serial.println("Image taken");
      break;
    default:
      // Serial.println("Scan image.....!");
      return p;
  }
  // OK success!
  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      // Serial.println("Image converted");
      break;
    default:
      // Serial.println("Error");
      return p;
  }
  // OK converted!
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    // Serial.println("Found a print match!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    // Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    // Serial.println("Did not find a match");
    beep(50);
    beep(50);
    beep(50);
    beep(50);
    return p;
  } else {
    // Serial.println("Unknown error");
    return p;
  }
  // found a match!
  // Serial.print("Found ID #"); Serial.print(finger.fingerID);
  // Serial.print(" with confidence of "); Serial.println(finger.confidence);
  runOpen();
  return finger.fingerID;
}
void runOpen(){
  beep(200);
  digitalWrite(relayPin,HIGH);
  timerID = timer.setTimeout(TIME_OPEN,handleTimer);
}
void handleTimer(){
  digitalWrite(relayPin,LOW);
}
