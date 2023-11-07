/*
  ESP32 Super low power Flower Pot soil moisture monitor  
  Uses the TTGO T-5 Epaper ESP32 Dev module as well as the widely available
  HD-38 Soil moisture Sensor and probes.  Minor mods to ESP32 board needed
  for full functionality as explained in the video
  https://www.youtube.com/c/jordanrubin6502
  2024 Jordan Rubin.
*/

#define BUTTON_PIN_BITMASK 0xC004
#define PIN_BITMASK 0x0004
#define BATTERYMON 35
#define BUTTON_A 15
#define BUTTON_B 14
#define SENSORPWR 22
#define SENSORSIG 13
#define USBPWRSENSE 2

#include <Arduino.h>
#include <SPIFFS.h>
#include <GxEPD2_BW.h>
#include "bitmaps.h"
#include <Fonts/FreeSansBold24pt7b.h>
#include <Fonts/FreeSerif9pt7b.h>
#include "GxEPD2_display_selection_new_style.h"

RTC_DATA_ATTR bool bootCount = 0;
RTC_DATA_ATTR int currentInt = 1;
RTC_DATA_ATTR int wet = 0;
RTC_DATA_ATTR int dry = 1000;
RTC_DATA_ATTR bool savebit = 0;
const char version[] = "1.0.2";
bool saveflag = 0;
double lastDebounceTime = 0;
unsigned long debounceDelay = 50; 

// getStatus ////////////////////////////////////////////////////////////
uint16_t getStatus(){
  digitalWrite(SENSORPWR,HIGH);
  delay(50);
  uint16_t sensval = analogRead(SENSORSIG);
  digitalWrite(SENSORPWR,LOW);
  return sensval;
}

// getBattery /////////////////////////////////////////////////////////// 
int getBattery(){
  uint16_t batteryVal = analogRead(BATTERYMON);
  float battery_voltage = ((float)batteryVal / 4095.0) * 2.0 * 3.3 * (1100 / 1000.0);   
  if (battery_voltage >= 4.17){ return 5;}
  if (battery_voltage >= 4.02){ return 4;}
  if (battery_voltage >= 3.87){ return 3;}
  if (battery_voltage >= 3.80){ return 2;}
  if (battery_voltage >= 3.73){ return 1;}  
  return 0;
}

// showStatus /////////////////////////////////////////////////////////// 
void showStatus(int status, int battery, int interval){
  uint16_t conv = map(status,wet,dry,10,0);
  status = conv;   
  display.setFullWindow();
  display.fillScreen(GxEPD_WHITE);
  display.drawRect(5,5,240,70,GxEPD_BLACK);
  display.drawRect(5,92,30,18,GxEPD_BLACK);
  display.drawRect(35,97,3,8,GxEPD_BLACK);
  display.drawCircle(232,100,13,GxEPD_BLACK);
  display.drawCircle(232,100,12,GxEPD_BLACK);
  display.drawLine(232,100,232,90,GxEPD_BLACK);
  display.drawLine(232,100,238,100,GxEPD_BLACK);
  int posx = 12;
  int cursory = 57;
  if (battery){
    cursory = 117;
    for (int i=0; i<10; i++){
      if (i < status){
        display.fillRoundRect(posx,10,18,60,7,GxEPD_BLACK);
      }
      else {
        display.drawRoundRect(posx,10,18,60,7,GxEPD_BLACK);
      }
    posx = posx + 23;
    }
  }
  posx = 8;
  for (int i=0; i<5; i++){
    if (i < battery){
      display.fillRect(posx,95,4,12,GxEPD_BLACK);
    }
    else {
      display.drawRect(posx,95,4,12,GxEPD_BLACK);    
    }
    posx = posx + 5;
  }
  char * comment = "FULL";
  if (status < 8){comment = "OK";}
  if (status < 6){comment = "HALF";}
  if (status < 4){comment = "LOW";}
  if (status < 2){comment = "ALARM!";}
  if (status < 1){comment = "EMPTY!";}
  if (battery == 0){comment = "CHARGE";}
  display.setFont(&FreeSansBold24pt7b);
  int16_t tbx, tby; uint16_t tbw, tbh;
  display.getTextBounds(comment, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t x = ((display.width() - tbw) / 2) - tbx;
  display.setCursor(x, cursory);
  display.print(comment);
  display.setFont(&FreeSerif9pt7b);
  display.setCursor(208, 105);
  display.print(interval);
  display.nextPage();
}

// intervalRefresh ////////////////////////////////////////////// 
void intervalRefresh(){
  display.setPartialWindow(88,32,40,48);
  display.fillScreen(GxEPD_WHITE);
  display.setCursor(95, 70);
  display.print(currentInt);
  display.nextPage();
}

// saveCfg ///////////////////////////////////////////////////////
void saveCfg() {
  File datafile = SPIFFS.open("/data.cfg","w");
  datafile.print(currentInt);
  datafile.print(',');
  datafile.print(wet);
  datafile.print(','); 
  datafile.print(dry);
  datafile.close();
  savebit = 1;
}

// setIntervalRoutine ////////////////////////////////////////////// 
void setIntervalRoutine(){
  display.drawCircle(45,60,40,GxEPD_BLACK);
  display.drawCircle(45,60,41,GxEPD_BLACK);
  display.fillCircle(45,60,2,GxEPD_BLACK);
  display.drawLine(45,60,45,23,GxEPD_BLACK);
  display.drawLine(45,60,70,60,GxEPD_BLACK);
  display.setFont(&FreeSerif9pt7b);
  display.setCursor(160, 90);
  display.print("Interval");
  display.setFont(&FreeSansBold24pt7b);
  display.setCursor(135, 70);
  display.print("hour");
  display.nextPage();
  intervalRefresh();
  unsigned long endTime = (millis() + 10000);
  while (millis() < endTime){
    int currentState = digitalRead(BUTTON_A);
    if ( (millis() - lastDebounceTime) > debounceDelay) {
      if (currentState == HIGH) {
        if (currentInt==8){currentInt=1;}
        else {currentInt=currentInt*2;}
        saveCfg();
        endTime = (millis() + 10000);
        lastDebounceTime = millis(); 
        intervalRefresh();
      }
    }
  }
}

// splashScreen //////////////////////////////////////////////////// 
void splashScreen(){
  bootCount = 1;
  display.drawBitmap(0,1,rtelogo90,104,90,GxEPD_BLACK);
  display.drawBitmap(130,0,qrcode,120,120,GxEPD_BLACK);
  display.setFont(&FreeSerif9pt7b);
  display.setCursor(7, 105);
  display.print("Soil Monitor");
  display.setCursor(32, 120);
  display.print(version);
  display.nextPage();
  delay(3000);
}

// TASK - wetButtonMonitor /////////////////////////////////////////
void IRAM_ATTR wetButtonMonitor(){
  if ((millis() - lastDebounceTime) > 200) {
    wet = wet+50;
    if (wet >= 2000){wet=0;}
    if (dry-wet < 200){dry=dry+50;}
    saveflag=1;
    lastDebounceTime = millis(); 
  } 
}

// TASK - dryButtonMonitor /////////////////////////////////////////
void IRAM_ATTR dryButtonMonitor(){
  if ((millis() - lastDebounceTime) > 200) {
    dry = dry+50;
    if (dry >= 4000){dry=1000;}
    if (dry-wet < 200){wet=dry-200;}
    saveflag=1;
    lastDebounceTime = millis(); 
  } 
}

// isCharging ///////////////////////////////////////////////////// 
bool isCharging(){
  bool usbCharge;
  int trydd = 0;
  while (trydd <6){
    usbCharge = digitalRead(USBPWRSENSE);
    if (!usbCharge){      
      delay(200);
      trydd++;
    }
    else {return 1;}
  }
  return 0;
}  

// usbChargeRoutine //////////////////////////////////////////////// 
void usbChargeRoutine(){
  bool isFull = false;
  display.fillScreen(GxEPD_WHITE);
  display.setPartialWindow(8,17,232,94);
  display.fillScreen(GxEPD_WHITE);
  display.setFont(&FreeSerif9pt7b);
  while (isCharging()){
    while ((isFull)&&(isCharging())){
      delay(1000);   
    }
    if (isFull){return;}
    display.fillScreen(GxEPD_WHITE);
    int posx = 16;
    display.drawRect(10,35,90,55,GxEPD_BLACK);
    display.drawRect(101,52,6,18,GxEPD_BLACK);
    for (int i=0; i<5; i++){
      if (i < getBattery()){
        display.fillRect(posx,42,12,41,GxEPD_BLACK);
      }
      else {
        display.drawRect(posx,42,12,41,GxEPD_BLACK);    
      } 
      display.fillRect(130,18,108,90,GxEPD_WHITE);
      display.setCursor(127, 67);
      if (getBattery() < 5){
        display.print("CHARGING...");
        display.nextPage();      
      }
      else {isFull = true;}       
      posx = posx + 16;
    }
    if (isFull == true){
      display.print("CHARGED.");
      display.nextPage();
    } 
  }
}

// calibrateRoutine //////////////////////////////////////////////// 
void calibrateRoutine(){
  bool usbCharge = digitalRead(USBPWRSENSE);
  attachInterrupt(BUTTON_A, wetButtonMonitor,HIGH);
  attachInterrupt(BUTTON_B, dryButtonMonitor,HIGH);
  display.fillScreen(GxEPD_WHITE);
  display.setPartialWindow(8,17,232,94);
  display.fillScreen(GxEPD_WHITE);
  unsigned long endTime = (lastDebounceTime + 20000);
  while (millis() < endTime){
    int posx = 16;
    display.fillScreen(GxEPD_WHITE);
    display.fillRect(130,18,108,90,GxEPD_WHITE);
    display.drawRect(8,40,110,50,GxEPD_BLACK);
    display.setFont(&FreeSerif9pt7b);
    display.setCursor(150, 33);
    display.print("WET: ");
    display.print(wet);
    display.setCursor(175, 67);
    display.print("SET");
    display.setCursor(150, 105);
    display.print("DRY: ");
    display.print(dry);
    display.setFont(&FreeSansBold24pt7b);
    display.setCursor(10, 80);
    display.print(getStatus());
    display.nextPage();
    posx = posx + 16;
    endTime = (lastDebounceTime + 20000);
  }
}

// restoreCfg ///////////////////////////////////////////////////////
void restoreCfg() {
  File file = SPIFFS.open("/data.cfg", "r");
  if (!file){saveCfg();}
  File f = SPIFFS.open( "/data.cfg", "r");
  size_t filesize = f.size();      
  char string[filesize+1];     
  f.read((uint8_t *)string, sizeof(string));
  f.close();
  string[filesize] = '\0';
  char* chars_array = strtok(string, ",");
  int i=0;
  while(chars_array){
    if (i==0){currentInt = atoi(chars_array);}
    else if (i==1){wet = atoi(chars_array);}
    else {dry = atoi(chars_array);}
    i++;
    chars_array = strtok (NULL, ",");
  }
}

// setup /////////////////////////////////////////////////////////// 
void setup() {
  SPIFFS.begin(true);
  btStop();
  pinMode(BUTTON_A,   INPUT);
  pinMode(BUTTON_B,   INPUT);
  pinMode(BATTERYMON, INPUT);
  pinMode(USBPWRSENSE,INPUT_PULLDOWN);
  pinMode(SENSORSIG,  INPUT);
  pinMode(SENSORPWR,  OUTPUT);
  digitalWrite(SENSORPWR,LOW);
  display.init(115200, true, 2, false);
  if (!savebit){                          //If config lost, restore from backup
    restoreCfg();                         
  }
  display.setRotation(1);
  display.fillScreen(GxEPD_WHITE);
  display.setFullWindow();
  display.setTextColor(GxEPD_BLACK);
  uint64_t GPIO_reason = esp_sleep_get_ext1_wakeup_status(); // Wakeup reasons
  if (GPIO_reason){  
    int theGpio = log(GPIO_reason)/log(2);
    if (theGpio == BUTTON_A){setIntervalRoutine();}   // Interval adjust pressed
    if (theGpio == BUTTON_B){calibrateRoutine();}     // Calibrate button pressed
    if (theGpio == USBPWRSENSE){usbChargeRoutine();}  // Charger Plugged in
  }
  else {
    if (bootCount < 1){splashScreen();}               // First boot, Splash Screen
  }
  if (saveflag){saveCfg();}
  showStatus(getStatus(),getBattery(),currentInt);    // Update Status and display main screen
  delay(500);                                         // Needed to complete final update, start sleep mode
  if (getBattery()){   
    int sleepSeconds = (currentInt*60)*60;                               // Charger, buttons and timer can wake up device
    uint64_t sleeptime = UINT64_C(sleepSeconds * 1000000);  
    esp_sleep_enable_timer_wakeup(sleeptime);
    esp_sleep_enable_ext1_wakeup(BUTTON_PIN_BITMASK,ESP_EXT1_WAKEUP_ANY_HIGH); //need resistor
    esp_deep_sleep_start();
  }
  else {                                               //Only Charger connection can wake up device
    esp_sleep_enable_ext1_wakeup(PIN_BITMASK,ESP_EXT1_WAKEUP_ANY_HIGH); //need resistor
    esp_deep_sleep_start();
  }
}

// loop ///////unused////////////////////////////////////////////// 
void loop() {}