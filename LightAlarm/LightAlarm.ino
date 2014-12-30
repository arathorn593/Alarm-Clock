#include <LiquidCrystal.h>
#include <Encoder.h>
#include "RTClib.h"
#include <Wire.h>

//Pin definitions
#define BLUE_PIN 9
#define GREEN_PIN 10
#define RED_PIN 11
#define LIGHT_PIN 0
#define BUTTON_PIN 13


//mode constants
#define TIME_MODE 0
#define MAIN_MENU 1

//Global variables
//mode/menu that is displayed currently
int mode = TIME_MODE;
int tickLen = 100;
int inactiveTime = 5000;
int ticksSinceMove = 0;
bool inactive = false;
int cursorRow = 0;
int cursorCol = 0;
int index = 0;
int menuLen = 5;
DateTime now;
bool buttonState = false;
bool oldButtonState = false;
bool lightState = false;
String displayText = "hello";


//LCD/Encoder setup
Encoder enc(2,3);
LiquidCrystal lcd(1, 4, 5, 6, 7, 8);
RTC_DS1307 rtc;

long oldPosition = 0;
long newPosition = 0;
long realPosition = 0;

//return the shift in the encoder since the last call of the function
int getEncShift(){
  //reset at the beginning instead of the end
  oldPosition = newPosition;
  //get actual value from the encoder
  realPosition = enc.read();
  //calculate the position with respect to the tactile feedback of the actual encoder
  newPosition = (realPosition + 1) / 4;  //calculate each increment based on
                                         //the position. Add 1 so that division
                                         //by 4 puts each increment in the middle
                                         //of a division and not at the beginning
  return (newPosition - oldPosition);
}

bool buttonPressed(){
  oldButtonState = buttonState;
  buttonState = (bool)digitalRead(BUTTON_PIN);
  return (buttonState && !oldButtonState);
  lcd.print(buttonState);
}

void onEncIncrement(){
  index = (index + 1) % menuLen;
}

void onEncDecrement(){
  if(index == 0){
    index = menuLen - 1;
  }else{
    index--;
  }
}


void onEncStill(){
  ticksSinceMove++;
  
  if((inactiveTime/tickLen) < ticksSinceMove){
      inactive = true;
  }
}

void onButton(){
  if(index == 0){
    lightState = !lightState;
  }    
}

void activity(){
  ticksSinceMove = 0;
  inactive = false;
}

void onTick(){
  displayText = "tick";
}

void displayDateTime(DateTime *now, int row, int col){

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
 
  //set cursor for date display
  lcd.setCursor(0, 1);
  
  switch(now->dayOfWeek()){
    case 0: lcd.print("Sun"); break;
    case 1: lcd.print("Mon"); break;
    case 2: lcd.print("Tue"); break;
    case 3: lcd.print("Wed"); break;
    case 4: lcd.print("Thu"); break;
    case 5: lcd.print("Fri"); break;
    case 6: lcd.print("Sat"); break; 
  }
  lcd.print(" ");
  
  switch(now->month()){
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
  
  if(now->day() < 10)
    lcd.print(" ");
  lcd.print(now->day());
  lcd.print(", ");
  
  lcd.print(now->year()); 
}

void gotoTime(){
  mode = TIME_MODE;
  lcd.cursor();
  menuLen = 2;
  index = 0;  
}

//goto the main menu
void gotoMain(){
  mode = MAIN_MENU;
  lcd.noCursor();
  
}

void drawTime(){
  //get date/time
  now = rtc.now();
  displayDateTime(&now, 0, 4);

  lcd.setCursor(0, 0);
  lcd.write("L");
  lcd.setCursor(15, 0);
  lcd.write("M");
  
  if(index == 0){
    //L selected
    cursorCol = 0;
    cursorRow = 0;
  }else if(index == 1){
    //M selected
    cursorCol = 15;
    cursorRow = 0;   
  }
  lcd.setCursor(cursorCol, cursorRow);
}

void drawLCD(){
  //clear lcd
  lcd.clear();
  switch(mode){
     case TIME_MODE:
       drawTime();
       break;
       
     case MAIN_MENU:
       break;
  }
  digitalWrite(LIGHT_PIN, lightState);
}

void setup() {
  lcd.begin(16, 2);
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(RED_PIN, OUTPUT);
  pinMode(LIGHT_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
  
  //rtc setup
  Wire.begin();
  rtc.begin();
  
  if(!rtc.isrunning()){
    //print message to LCD
    lcd.print("RTC NOT Running");
    //sets rtc to date and time of sketch compile
    rtc.adjust(DateTime(__DATE__, __TIME__));
  }
  
  gotoTime();
}

void loop() {
  onTick();
  //update encoder and take appropriate action
  int encChange = getEncShift();
  if(encChange > 0){
    activity();
    onEncIncrement();
  }else if(encChange < 0){
    activity();
    onEncDecrement();
  }else if(buttonPressed()){
    onButton();
  }else{
    onEncStill();
  }
  
  drawLCD();
   
  //delay fo rthe lcd to update
  delay(tickLen);
}
