#define uS_TO_S_FACTOR 1000000  
#define TIME_TO_SLEEP 60 
#define BUTTON_PIN_BITMASK 0x8000

#include <Arduino.h>
#include <GxEPD2_BW.h>
#include "bitmaps.h"
#include <Fonts/FreeSansBold24pt7b.h>
#include <Fonts/FreeSerif9pt7b.h>
#include "GxEPD2_display_selection_new_style.h"

const char version[] = "0.0.1a";
RTC_DATA_ATTR int bootCount = 0;
RTC_DATA_ATTR int currentInt = 1;

int getStatus(){
  return 60;
}

int getBattery(){
  return 3;
}

int getInterval(){
  return 4;
}

// showStatus /////////////////////////////////////////////////////////// 
void showStatus(int status, int battery, int interval){
  status = status / 10;
  display.fillScreen(GxEPD_WHITE);
  display.drawRect(5,5,240,70,GxEPD_BLACK);
  display.drawRect(5,92,30,18,GxEPD_BLACK);
  display.drawRect(35,97,3,8,GxEPD_BLACK);
  display.drawCircle(232,100,13,GxEPD_BLACK);
  display.drawCircle(232,100,12,GxEPD_BLACK);
  display.drawLine(232,100,232,90,GxEPD_BLACK);
  display.drawLine(232,100,238,100,GxEPD_BLACK);
  int posx = 12;
  for (int i=0; i<10; i++){
    if (i < status){
      display.fillRoundRect(posx,10,18,60,7,GxEPD_BLACK);
    }
    else {
      display.drawRoundRect(posx,10,18,60,7,GxEPD_BLACK);
    }
    posx = posx + 23;
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
  if (status < 8){
    comment = "OK";
  }
  if (status < 6){
    comment = "HALF";
  }
  if (status < 4){
    comment = "LOW";
  }
  if (status < 2){
    comment = "WARNING!";
  }
  if (status < 1){
    comment = "EMPTY!!!!";
  }
  display.setFont(&FreeSansBold24pt7b);
  int16_t tbx, tby; uint16_t tbw, tbh;
  display.getTextBounds(comment, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t x = ((display.width() - tbw) / 2) - tbx;
  uint16_t y = ((display.height() - tbh) / 2) - tby;
  display.setCursor(x, 117);
  display.print(comment);
  display.setFont(&FreeSerif9pt7b);
  display.setCursor(208, 105);
  display.print(interval);
  display.nextPage();
}

// setup /////////////////////////////////////////////////////////// 
void setup() {
  Serial.begin(115200);
  display.init(115200, true, 2, false);
  display.setRotation(1);
  display.fillScreen(GxEPD_WHITE);
  display.setFullWindow();
  display.setFont(&FreeSerif9pt7b);
  display.setTextColor(GxEPD_BLACK);
  display.firstPage();
  ++bootCount;
  if (bootCount <= 1){
    display.drawBitmap(0,1,rtelogo90,104,90,GxEPD_BLACK);
    display.drawBitmap(130,0,qrcode,120,120,GxEPD_BLACK);
    display.setCursor(7, 105);
    display.print("Soil Monitor");
    display.setCursor(32, 120);
    display.print(version);
    display.nextPage();
    sleep(1);
  }
  showStatus(getStatus(),getBattery(),getInterval());

   
   
 sleep(2);


    
  uint64_t GPIO_reason = esp_sleep_get_ext1_wakeup_status();
  Serial.print("GPIO that triggered the wake up: GPIO ");
  Serial.println((log(GPIO_reason))/log(2), 0);



  display.fillScreen(GxEPD_WHITE);
  display.setCursor(50, 50);
  display.println("Boot number: " + String(bootCount));
  display.nextPage();
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  esp_sleep_enable_ext1_wakeup(BUTTON_PIN_BITMASK,ESP_EXT1_WAKEUP_ANY_HIGH); //need resistor

  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) +
  " Seconds");


  Serial.println("Going to sleep now");
  delay(1000);
  Serial.flush(); 
  esp_deep_sleep_start();

}

// loop /////////////////////////////////////////////////////////// 
void loop() {
}

