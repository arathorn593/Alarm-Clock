#include <LiquidCrystal.h>
#include "RTClib.h"
#include <Wire.h>

RTC_DS1307 rtc;

LiquidCrystal lcd(1, 4, 5, 6, 7, 8);

void displayTime(DateTime *now, int row, int col){

  //account for the hour only being one digit
  if(now->hour() < 10){
    lcd.setCursor(col + 1, row);
  }else{
    lcd.setCursor(col, row);
  }
  lcd.print(now->hour()); lcd.print(":");
  
  //account for the minute being only 1 digit
  if(now->minute() < 10){
    lcd.print("0");
  }
  lcd.print(now->minute()); lcd.print(":");

  //account for the seconds only being one digit
  if(now->second() < 10){
    lcd.print("0");
  }
  lcd.print(now->second());
  
}
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

DateTime now;
void loop(){
  lcd.setCursor(0, 0);
  //get date/time
  now = rtc.now();
  
  //displaying date/time
  lcd.print("R");//print characters for selecting color to edit
  lcd.print("G");
  lcd.print("B");
  
  displayTime(&now, 0, 5);

  lcd.setCursor(15, 0);
  lcd.print("M");
  
  //set cursor for date display
  lcd.setCursor(0, 1);
  
  switch(now.dayOfWeek()){
    case 0: lcd.print("Sun"); break;
    case 1: lcd.print("Mon"); break;
    case 2: lcd.print("Tue"); break;
    case 3: lcd.print("Wed"); break;
    case 4: lcd.print("Thu"); break;
    case 5: lcd.print("Fri"); break;
    case 6: lcd.print("Sat"); break; 
  }
  lcd.print(" ");
  
  switch(now.month()){
    case 0: lcd.print("Jan"); break;
    case 1: lcd.print("Feb"); break;
    case 2: lcd.print("Mar"); break;
    case 3: lcd.print("Apr"); break;
    case 4: lcd.print("May"); break;
    case 5: lcd.print("Jun"); break;
    case 6: lcd.print("Jul"); break;
    case 7: lcd.print("Aug"); break;
    case 8: lcd.print("Sep"); break;
    case 9: lcd.print("Oct"); break;
    case 10: lcd.print("Nov"); break;
    case 11: lcd.print("Dec"); break;
  }
  lcd.print(" ");
  
  if(now.day() < 10)
    lcd.print(" ");
  lcd.print(now.day());
  lcd.print(", ");
  
  lcd.print(now.year());
  
  delay(1000);
}
