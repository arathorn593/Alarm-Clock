#include <LiquidCrystal.h>
#include "RTClib.h"
#include <Wire.h>

RTC_DS1307 rtc;

LiquidCrystal lcd(1, 4, 5, 6, 7, 8);

void setup(){
//LCD setup
  //set up the LCD's col and rows
  lcd.begin(16, 2);
  
//rtc setup
  Wire.begin();
  rtc.begin();
  
  if(!rtc.isrunning()){
    //print message to LCD
    lcd.print("RTC NOT Running");
    //sets rtc to date and time of sketch compile
    rtc.adjust(DateTime(__DATE__, __TIME__));
  }
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
}

void loop(){
  lcd.setCursor(0, 0);
  
  DateTime now = rtc.now();
  
  char time[9];
  
  snprintf(time, 9, "%d:%d:%d", now.hour(), now.minute(), now.second());
  
  lcd.print(time);
  
  delay(1000);
}
