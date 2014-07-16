#include <Encoder.h>
#include <LiquidCrystal.h>
#include "RTClib.h"
#include <Wire.h>

#define BLUE_PIN 9
#define GREEN_PIN 10
#define RED_PIN 11

#define FLASH_LEN 5 //in tenths of a second
boolean flashState = false; //state (on/off) of flashing letter
int flashCounter = 0; //counts up ticks and then changes flash state
int clkIndex = 0; //index of which letter is flashing
                    //R = 0; G = 1; B = 2; M = 3
#define L 0
#define M 1

boolean idle = false;
unsigned long currentMillis = 0;
unsigned long previousMillis = 0;
#define IDLE_TIME 5000 //milliseconds of inactivity it takes to stop blinking letter

#define CLK_INDEX_LEN 2
long oldPosition = 0;
long newPosition = 0;
long realPosition= 0;

RTC_DS1307 rtc;
Encoder enc(2, 3);
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
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(RED_PIN, OUTPUT);
}

DateTime now;
void loop(){
  //check if idle
  currentMillis = millis();
  if(currentMillis - previousMillis > IDLE_TIME){
    idle = true;
    flashState = 1; //change to one so all letters are present
    previousMillis = currentMillis;
  }
  //update encoder
  realPosition = enc.read();
  newPosition = (realPosition + 1) / 4;  //calculate each increment based on
                                         //the position. Add 1 so that division
                                         //by 4 puts each increment in the middle
  //update index                         //of a division and not at the beginning
  if(newPosition != oldPosition){
/*
    //wrap around index if it is out of bounds
    if(newPosition > oldPosition){
      if(clkIndex >= CLK_INDEX_LEN - 1){
        clkIndex = 0;
      }else{
        clkIndex++;
      }
      

    }else{
      if(clkIndex <= 0){
        clkIndex = CLK_INDEX_LEN -1;
      }else{
        clkIndex--;
      } 
    }
    */
    if(idle == false)
      clkIndex = 1 - clkIndex;
      
    oldPosition = newPosition;
    flashState = false; //set character to be off so user knows move has happened
    flashCounter = 0;
    previousMillis = currentMillis; //updating for idle calculation
    idle = false; //the user is not idling anymore
  }
   
  //update state variable
  if(!idle){
    flashCounter++;
    if(flashCounter >= FLASH_LEN){
      flashCounter = 0;
      flashState = !flashState;
    }
  }
  
  lcd.setCursor(0, 0);
  //print characters for menu and color selection
  if(!flashState){
    if(clkIndex == 0){
      lcd.print(" ");
    }else{
      lcd.print("L");
    }
    lcd.setCursor(15, 0);
    if(clkIndex == 1){
      lcd.print(" ");
    }else{
      lcd.print("M");
    }
  }else{
    lcd.print("L");
    lcd.setCursor(15, 0);
    lcd.print("M");
  }
  
  //get date/time
  now = rtc.now();
  displayTime(&now, 0, 4);

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
  
  delay(100);
}
