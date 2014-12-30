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

//define menu indexes
#define CLOCK_INDEX 0
#define ALARMS_INDEX 1
#define RED_INDEX 2
#define GREEN_INDEX 3
#define BLUE_INDEX 4

//mode constants
#define TIME_MODE 0
#define MAIN_MENU 1
#define ALARMS _MENU 2

#define ALARM_SIZE 5 //the number of bytes in the struct (used to increment loop)
typedef struct{
  byte light; //yes/no in byte index 0
  byte days; //first bit = nothing, days procede from there
  byte hour; //decimal hour(24)
  byte minutes; //decimal
  byte timeLight; //decimal, time in minutes before alarm
}Alarm;

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
int redVal = 255;
int blueVal = 255;
int greenVal = 255;
bool selected = false; //is the item at the pointer selected in the main menu

int numAlarms = 0;

#define MENU_SIZE 5
char * menuItems[] = {
  "Clock",
  "Alarms",
  "Red",
  "Green",
  "Blue"
};

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
}

int changeColor(int colorVal, int change){
  colorVal += change;
  if(colorVal > 255){
    colorVal = 255;
  }else if(colorVal < 0){
    colorVal = 0;
  }
  return colorVal;
}

void changeSelected(int change){
  switch(index){
    case RED_INDEX:
      redVal = changeColor(redVal, change);
      break;
      
    case GREEN_INDEX:
      greenVal = changeColor(greenVal, change);
      break;
      
    case BLUE_INDEX:
      blueVal = changeColor(blueVal, change);
      break;
  }
}

void onEncIncrement(int change){
  if(selected){
    changeSelected(change);
  }else{
    index = (index + 1) % menuLen;
  }
}

void onEncDecrement(int change){
  if(selected){
    changeSelected(change);
  }else{
    if(index == 0){
      index = menuLen - 1;
    }else{
      index--;
    }
  }
}


void onEncStill(){
  ticksSinceMove++;
  
  if((inactiveTime/tickLen) < ticksSinceMove){
      inactive = true;
  }
}

void onTimeButton(){
  if(index == 0){
    lightState = !lightState;
  }else if(index == 1){
    gotoMain();
  }
}



void onMainButton(){
  if(selected){
    selected = false;
  }else{
    switch(index){
      case CLOCK_INDEX:
        gotoTime();
        break;
      
      case ALARMS_INDEX:
        gotoAlarms();
        break;
       
      default:  //it is one of the colors (change if more added)
        selected = true;
        
    }
  }
}

void onButton(){
  switch(mode){
    case TIME_MODE:
      onTimeButton();
      break;
    case MAIN_MENU:
      onMainButton();
      break;
  }
}

void activity(){
  ticksSinceMove = 0;
  inactive = false;
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
  menuLen = MENU_SIZE;
  index = 0;
}

//goto the alarm menu
void gotoAlarms(){
  mode = ALARMS_MENU;
  lcd.noCursor();
  menuLen = numAlarms;
  index = 0;
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

void printMenuItem(int index, int row){
  lcd.print(menuItems[index]);
  //move cursor to end (right adjust setting val)
  lcd.setCursor(13, row);
  if(menuItems[index] == "Red"){
    lcd.print(redVal);
  }else if(menuItems[index] == "Green"){
    lcd.print(greenVal);
  }else if(menuItems[index] == "Blue"){
    lcd.print(blueVal);
  }
}

void drawMain(){  
  //draw pointer
  lcd.write(">");
  //draw first item
  lcd.setCursor(1, 0);
  printMenuItem(index, 0);
  //draw second item
  lcd.setCursor(1, 1);
  printMenuItem((index + 1) % menuLen, 1); //print next item (w/ wraparound)
  
  //move cursor back to pointer
  lcd.home();
  //blink cursor if the item is selected
  if(selected){
    lcd.blink();
  }else{
    lcd.noBlink();
  }
  
}

void setColors(){
  //set colors. invert values because backlight led's weird (common anode)
  analogWrite(RED_PIN, 255-redVal);
  analogWrite(GREEN_PIN, 255-greenVal);
  analogWrite(BLUE_PIN, 255-blueVal);
}

void drawLCD(){
  //update the colors of the backlight
  setColors();
  //clear lcd
  lcd.clear();
  switch(mode){
     case TIME_MODE:
       drawTime();
       break;
       
     case MAIN_MENU:
       drawMain();
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
  //update encoder and take appropriate action
  int encChange = getEncShift();
  if(encChange > 0){
    activity();
    onEncIncrement(encChange);
  }else if(encChange < 0){
    activity();
    onEncDecrement(encChange);
  }else if(buttonPressed()){
    onButton();
  }else{
    onEncStill();
  }
  
  drawLCD();
   
  //delay fo rthe lcd to update
  delay(tickLen);
}
