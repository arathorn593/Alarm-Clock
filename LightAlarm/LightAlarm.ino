#include <Encoder.h>
#include <RTClib.h>
#include <LiquidCrystal.h>
#include <Wire.h>
#include <EEPROM.h>

//Pin definitions
#define BLUE_PIN 11
#define GREEN_PIN 10
#define RED_PIN 9
#define LIGHT_PIN 13
#define BUTTON_PIN 4
#define BUZZER_PIN 5

#define LCD_ROWS 2
#define LCD_COLS 16

//EEPROM
#define MEM_SIZE 1024
#define ALARM_NUM  100 //number of alarms
#define ALARM_START 4 //byte index where the first alarm is

#define RED_LCD_INDEX 0
#define GREEN_LCD_INDEX 1
#define BLUE_LCD_INDEX 2
#define SNOOZE_MEM_INDEX 3


//define menu indexes
#define CLOCK_INDEX 0
#define ALARMS_INDEX 1
#define RED_INDEX 2
#define GREEN_INDEX 3
#define BLUE_INDEX 4
#define SNOOZE_INDEX 5
#define HOUR_INDEX 6
#define MINUTE_INDEX 7
#define SECOND_INDEX 8
#define DAY_INDEX 9
#define MONTH_INDEX 10
#define YEAR_INDEX 11

//mode constants
#define TIME_MODE 0
#define MAIN_MENU 1
#define ALARMS_MENU 2
#define ALARM_EDIT 3

#define ALARM_SIZE 6 //the number of bytes in the struct (used to increment loop)
typedef struct{
  byte active; //boolean 1 or 0 in index 0
  byte light; //boolean 1 or 0 in index 0
  byte days; //days from sunday to saturday in indecies 0-6
  byte hrs; //decimal hour(24)
  byte minutes; //decimal
  byte timeLight; //decimal, time in minutes before alarm
}Alarm;

//the offsets for each alarm part in memory
#define ACTIVE_OFFSET 0
#define LIGHT_OFFSET 1
#define DAYS_OFFSET 2
#define HRS_OFFSET 3
#define MINUTES_OFFSET 4
#define TIME_LIGHT_OFFSET 5

#define NUM_ALARM_SETTINGS 13
//constants for indexes of alarm elements
#define BACK_INDEX 0
#define ACTIVE_INDEX 1
#define LIGHT_INDEX 2
#define HOURS_INDEX 3
#define MINUTES_INDEX 4
#define LIGHT_TIME_INDEX 5
#define SUN_INDEX 6
#define MON_INDEX 7
#define TUES_INDEX 8
#define WED_INDEX 9
#define THURS_INDEX 10
#define FRI_INDEX 11
#define SAT_INDEX 12

//alarm constants
#define BUZZ_LEN 500
#define QUIET_LEN 300
#define BUZZ_FREQ 483

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
byte redVal = 255;
byte blueVal = 255;
byte greenVal = 255;
bool selected = false; //is the item at the pointer selected in the main menu
Alarm editAlarm;
int editAlarmIndex;
const int numAlarms = 10;
Alarm alarms[numAlarms];
byte oldMinute = 60;
bool alarmState = false;
byte cursorPos = 0; //0 is top left 31 is bottom right. 
#define SNOOZE_POS 0
byte alarmGoalPos = LCD_COLS + 1; //the second char in the second row
unsigned long lastBuzzTime = 0;
int curAlarm = -1;
int snoozes[numAlarms];
int snoozeTime = 7;
int goalMoveCount = 0;
#define GOAL_MOVE_FREQ 4 //how many ticks before the goal moves

#define MENU_SIZE 12
char * menuItems[] = {
  "Clock",
  "Alarms",
  "Red",
  "Green",
  "Blue", 
  "Snooze",
  "Hour",
  "Minute",
  "Second",
  "Day",
  "Month",
  "Year"
};

//LCD/Encoder setup
Encoder enc(3, 2);
LiquidCrystal lcd(1, 0, 6, 7, 8, 12);
RTC_DS3231 rtc;

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

void loadData(){
  redVal = EEPROM.read(RED_LCD_INDEX);
  greenVal = EEPROM.read(GREEN_LCD_INDEX);
  blueVal = EEPROM.read(BLUE_LCD_INDEX);
  snoozeTime = EEPROM.read(SNOOZE_MEM_INDEX);
  
  for(int i = 0; i < numAlarms; i++){
    int index = i*ALARM_SIZE + ALARM_START;
    alarms[i].active = EEPROM.read(index+ACTIVE_OFFSET);
    alarms[i].light = EEPROM.read(index+LIGHT_OFFSET);
    alarms[i].days = EEPROM.read(index+DAYS_OFFSET);
    alarms[i].hrs = EEPROM.read(index+HRS_OFFSET);
    alarms[i].minutes = EEPROM.read(index+MINUTES_OFFSET);
    alarms[i].timeLight = EEPROM.read(index+TIME_LIGHT_OFFSET);
  }
}

void writeEEPROM(byte value, int index){
  if(EEPROM.read(index) != value){
    EEPROM.write(index, value);
  }
}

void saveData(){
  writeEEPROM(redVal, RED_LCD_INDEX);
  writeEEPROM(greenVal, GREEN_LCD_INDEX);
  writeEEPROM(blueVal, BLUE_LCD_INDEX);
  writeEEPROM(snoozeTime, SNOOZE_MEM_INDEX);
  
  for(int i = 0; i < numAlarms; i++){
    int index = i*ALARM_SIZE + ALARM_START;
    writeEEPROM(alarms[i].active, index+ACTIVE_OFFSET);
    writeEEPROM(alarms[i].light, index+LIGHT_OFFSET);
    writeEEPROM(alarms[i].days, index+DAYS_OFFSET);
    writeEEPROM(alarms[i].hrs, index+HRS_OFFSET);
    writeEEPROM(alarms[i].minutes, index+MINUTES_OFFSET);
    writeEEPROM(alarms[i].timeLight, index+TIME_LIGHT_OFFSET);
  }
}

byte changeVal(byte val, int change, int minimum, int maximum){
  int newVal = int(val);
  newVal += change;
  if(newVal > maximum){
    newVal = (newVal - (maximum + 1)) + minimum;
  }else if(newVal < minimum){
    newVal = (newVal - minimum) + (maximum + 1);
  }
  return byte(newVal);
}

void mainChangeSelected(int change){
  switch(index){
      case RED_INDEX:
        redVal = changeVal(redVal, change, 0, 255);
        break;
        
      case GREEN_INDEX:
        greenVal = changeVal(greenVal, change, 0, 255);
        break;
        
      case BLUE_INDEX:
        blueVal = changeVal(blueVal, change, 0, 255);
        break;
      
      case SNOOZE_INDEX:
        snoozeTime = changeVal(snoozeTime, change, 0, 255);
        break;
        
      case HOUR_INDEX:
      {
        byte newHour = changeVal(byte(now.hour()), change, 0, 23);
        rtc.adjust(DateTime(now.year(), now.month(), now.day(), newHour, now.minute(), now.second()));
        break;
      } 
      case MINUTE_INDEX:
      {
        byte newMinute = changeVal(byte(now.minute()), change, 0, 59);
        rtc.adjust(DateTime(now.year(), now.month(), now.day(), now.hour(), newMinute, now.second()));
        break;
      }  
      case SECOND_INDEX:
        //set the seconds to 0
        rtc.adjust(DateTime(now.year(), now.month(), now.day(), now.hour(), now.minute(), 0));
        break;
      case DAY_INDEX:
      {
        byte maxDays = 0;
        if(now.month() == 1){
          maxDays = 28;
        }else if(now.month() == 3 || now.month() == 5 || now.month() == 8 || now.month() == 10){
          maxDays = 30;  
        }else{
          maxDays = 31;
        }
        
        byte newDay = changeVal(byte(now.day()), change, 0, maxDays);
        rtc.adjust(DateTime(now.year(), now.month(), newDay, now.hour(), now.minute(), now.second()));
        break;
      }
      case MONTH_INDEX:
      {
        byte newMonth = changeVal(byte(now.month()), change, 1, 12);
        rtc.adjust(DateTime(now.year(), newMonth, now.day(), now.hour(), now.minute(), now.second()));
        break;
      }
      case YEAR_INDEX:
      {
        int newYear = changeVal(byte(now.year()-2000), change, 0, 255);
        rtc.adjust(DateTime(newYear, now.month(), now.day(), now.hour(), now.minute(), now.second()));
        break;
      }
  }
}

byte flipBit(byte val, int index){
  if(bitRead(val, index) == 1){
    bitWrite(val, index, 0);
  }else{
    bitWrite(val, index, 1);
  }
  return val;    
}

void editChangeSelected(int change){
  switch(index){
      case ACTIVE_INDEX:
        editAlarm.active = flipBit(editAlarm.active, 0);
        break;
      case LIGHT_INDEX:
        editAlarm.light = flipBit(editAlarm.light, 0);
        break;
      case HOURS_INDEX:
        editAlarm.hrs = changeVal(editAlarm.hrs, change, 0, 23);
        break;
      case MINUTES_INDEX:
        editAlarm.minutes = changeVal(editAlarm.minutes, change, 0, 59);
        break;
      case LIGHT_TIME_INDEX:
        editAlarm.timeLight = changeVal(editAlarm.timeLight, change, 0, 255);
        break;
      //must be a day of the week
      default:
        //convert index to day index
        int dayIndex = index - SUN_INDEX;
        editAlarm.days = flipBit(editAlarm.days, dayIndex);  
  }
}

void changeSelected(int change){
  if(mode == MAIN_MENU){
    mainChangeSelected(change);
  }else if(mode == ALARM_EDIT){
    editChangeSelected(change);
  }
}

void onEncIncrement(int change){
  if(selected){
    changeSelected(change);
  }else if(alarmState){
    cursorPos = changeVal(cursorPos, change, 0, LCD_COLS*LCD_ROWS-1);
  }else{
    index = (index + 1) % menuLen;
  }
}

void onEncDecrement(int change){
  if(selected){
    changeSelected(change);
  }else if(alarmState){
    cursorPos = changeVal(cursorPos, change, 0, LCD_COLS*LCD_ROWS-1);
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
    if(index == RED_INDEX || index == GREEN_INDEX || index == BLUE_INDEX || index == SNOOZE_INDEX){
      saveData();
    }
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

void onAlarmsButton(){
  if(index == 0){
    gotoMain();
  }else{
    gotoEdit(index - 1);
  }
}

void onEditButton(){
  if(selected){
    selected = false;
  }else{
    if(index == BACK_INDEX){
      alarms[editAlarmIndex] = editAlarm;
      saveData();
      gotoAlarms();
    }else{
      selected = true;
    }
  }
}

void snoozeAlarm(){
  snoozes[curAlarm] = (getDayMin(now.hour(), now.minute()) + snoozeTime) % 
                       (60 * 24);
  alarmState = false;
  if(mode!= TIME_MODE){
    lcd.noCursor();
  }
}

void turnOffAlarm(){
  alarmState = false;
  snoozes[curAlarm] = -1;
  if(mode != TIME_MODE){
    lcd.noCursor();
  }
}

void onAlarmButton(){
  if(cursorPos == SNOOZE_POS){
    snoozeAlarm();
  }else if(cursorPos == alarmGoalPos){
    turnOffAlarm();
  }
}

void onButton(){
  if(alarmState){
    onAlarmButton();
  }else{
    switch(mode){
      case TIME_MODE:
        onTimeButton();
        break;
      case MAIN_MENU:
        onMainButton();
        break;
      case ALARMS_MENU:
        onAlarmsButton();
        break;      
      case ALARM_EDIT:
        onEditButton();
        break;
    }
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
  
  switch(now->dayOfTheWeek()){
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
    case 1: lcd.print("Jan"); break;
    case 2: lcd.print("Feb"); break;
    case 3: lcd.print("Mar"); break;
    case 4: lcd.print("Apr"); break;
    case 5: lcd.print("May"); break;
    case 6: lcd.print("Jun"); break;
    case 7: lcd.print("Jul"); break;
    case 8: lcd.print("Aug"); break;
    case 9: lcd.print("Sep"); break;
    case 10: lcd.print("Oct"); break;
    case 11: lcd.print("Nov"); break;
    case 12: lcd.print("Dec"); break;
    default: lcd.print(now->month()); break;
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
  menuLen = numAlarms + 1; //extra for menu entry
  index = 0;
}

void gotoEdit(int alarmIndex){
  mode = ALARM_EDIT;
  lcd.noCursor();
  menuLen = NUM_ALARM_SETTINGS;
  index = 0;
  editAlarm = alarms[alarmIndex];
  editAlarmIndex = alarmIndex;
}

void drawTime(){
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
  
  if(menuItems[index] == "Red"){
    printNum(redVal, row);
  }else if(menuItems[index] == "Green"){
    printNum(greenVal, row);
  }else if(menuItems[index] == "Blue"){
    printNum(blueVal, row);
  }else if(menuItems[index] == "Snooze"){
    printNum(snoozeTime, row);
  }else if(menuItems[index] == "Hour"){
    printNum(byte(now.hour()), row);
  }else if(menuItems[index] == "Minute"){
    printNum(byte(now.minute()), row);
  }else if(menuItems[index] == "Second"){
    printNum(byte(now.second()), row);
  }else if(menuItems[index] == "Day"){
    printNum(byte(now.day()), row);
  }else if(menuItems[index] == "Month"){
    lcd.setCursor(13, row);
    switch(now.month()){
      case 1: lcd.print("Jan"); break;
      case 2: lcd.print("Feb"); break;
      case 3: lcd.print("Mar"); break;
      case 4: lcd.print("Apr"); break;
      case 5: lcd.print("May"); break;
      case 6: lcd.print("Jun"); break;
      case 7: lcd.print("Jul"); break;
      case 8: lcd.print("Aug"); break;
      case 9: lcd.print("Sep"); break;
      case 10: lcd.print("Oct"); break;
      case 11: lcd.print("Nov"); break;
      case 12: lcd.print("Dec"); break;
      default: lcd.print("Err"); break;
    }
  }else if(menuItems[index] == "Year"){
    lcd.setCursor(12, row);
    lcd.print(now.year());
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

void printAlarmOnOff(int index, int row){
  if(alarms[index].active != 0){
    int start = 9;
    char days[] = "SMTWRFS";
    for(int i = 0; i < 7; i++){
      lcd.setCursor(start + i, row);
      if(bitRead(alarms[index].days, i) == 1){
        lcd.write(days[i]);
      }
    }
  }else{
    lcd.setCursor(13, row);
    lcd.print("Off");
  }
}

void printAlarm(int index, int row){
  lcd.setCursor(1, row);
  if(index == 0){
    lcd.print("Menu");
  }else{
    index -= 1; //to get in range of alarms list
    //print an extra zero if needed
    if(alarms[index].hrs < 10)
      lcd.print("0");
    lcd.print(alarms[index].hrs);
    
    lcd.print(":");
    
    if(alarms[index].minutes < 10)
      lcd.print("0");
    lcd.print(alarms[index].minutes);
    
    printAlarmOnOff(index, row);
  }
}

void drawAlarms(){
  //draw pointer
  lcd.write(">");
  //drawfirst item in row 0
  printAlarm(index, 0);
  //draw second item in row 1
  printAlarm((index + 1) % menuLen, 1);
}

void printOnOff(byte val, int index, int row){
  if(bitRead(val, index) == 1){
    lcd.setCursor(14, row);
    lcd.print("On");
  }else{
    lcd.setCursor(13, row);
    lcd.print("Off");
  } 
}

void printNum(byte val, int row){
  if(val >= 100){
    lcd.setCursor(13, row);
  }else if(val >= 10){
    lcd.setCursor(14, row);
  }else{
    lcd.setCursor(15, row);
  }
  lcd.print(val); 
}

void printEditItem(int index, int row){
  lcd.setCursor(1, row);
  switch(index){
    case BACK_INDEX:
      lcd.print("Back");
      break;
    case ACTIVE_INDEX:
      lcd.print("Alarm?");
      printOnOff(editAlarm.active, 0, row);
      break;
    case LIGHT_INDEX:
      lcd.print("Light?");
      printOnOff(editAlarm.light, 0, row);
      break;
    case HOURS_INDEX:
      lcd.print("Hour");
      printNum(editAlarm.hrs, row);
      break;
    case MINUTES_INDEX:
      lcd.print("Minute");
      printNum(editAlarm.minutes, row);  
      break;
    case LIGHT_TIME_INDEX:
      lcd.print("Mins b4 lt");
      printNum(editAlarm.timeLight, row);
      break;
    //must be a day of the week
    default:
      //convert index to day index
      index -= SUN_INDEX;
      switch(index){
        case 0:
          lcd.print("Sunday");
          break;
        case 1:
          lcd.print("Monday");
          break;
        case 2:
          lcd.print("Tuesday");
          break;
        case 3:
          lcd.print("Wednesday");
          break;
        case 4:
          lcd.print("Thursday");
          break;
        case 5:
          lcd.print("Friday");
          break;
        case 6:
          lcd.print("Saturday");
          break;
      }
      //print if the day is on or off
      printOnOff(editAlarm.days, index, row);
  }
}

void drawEdit(){
  lcd.write(">");
  //drawFirstItem in row 0
  printEditItem(index, 0);
  //draw second item in row 1
  printEditItem((index + 1) % menuLen, 1);
  
  //move cursor back to pointer
  lcd.home();
  //blink cursor if the item is selected
  if(selected){
    lcd.blink();
  }else{
    lcd.noBlink();
  }
}

int getCol(int pos){
  while(pos >= LCD_COLS){
    pos -= LCD_COLS;
  }
  return pos;
}

int getRow(int pos){
  int row = 0;
  while(pos >= LCD_COLS){
    row++;
    pos -= LCD_COLS;
  }
  return row;
}

void drawAlarm(){
  lcd.print("S ");
  lcd.print(now.hour());
  lcd.print(":");
  lcd.print(now.minute());
  lcd.print(" ");
  lcd.print(now.month());
  lcd.print("/");
  lcd.print(now.day());
  lcd.print("/");
  lcd.print(now.year()-2000);
  lcd.setCursor(getCol(alarmGoalPos - 1), getRow(alarmGoalPos - 1));
  lcd.write('>');
  lcd.setCursor(getCol(alarmGoalPos + 1), getRow(alarmGoalPos + 1));
  lcd.write('<');
  
  lcd.setCursor(getCol(cursorPos), getRow(cursorPos));
}

void setColors(){
  //set colors. invert values because backlight led's weird (common anode)
  analogWrite(RED_PIN, 255-redVal);
  analogWrite(GREEN_PIN, 255-greenVal);
  analogWrite(BLUE_PIN, 255-blueVal);
}

void soundBuzzer(){
  //account for overflow
  if(millis() < lastBuzzTime){
    lastBuzzTime = millis();
  }
  if(millis() - lastBuzzTime > QUIET_LEN + BUZZ_LEN){
    tone(BUZZER_PIN, BUZZ_FREQ, BUZZ_LEN);
    lastBuzzTime = millis();
  }
}

void drawLCD(){
  //update the colors of the backlight
  setColors();
  //clear lcd
  lcd.clear();
    if(alarmState){
    soundBuzzer();
    drawAlarm();
  }else{
    lastBuzzTime = 0;
    noTone(BUZZER_PIN);
    switch(mode){
       case TIME_MODE:
         drawTime();
         break;
         
       case MAIN_MENU:
         drawMain();
         break;
       
       case ALARMS_MENU:
         drawAlarms();
         break;
         
       case ALARM_EDIT:
         drawEdit();
         break;
    }
  }
  digitalWrite(LIGHT_PIN, lightState);
  

}

int getDayMin(byte hr, byte mn){
  return hr*60 + mn; 
}

void checkAlarms(){
   int curMin = getDayMin(now.hour(), now.minute());
   int alarmMin = -1;
   for(int i = 0; i < numAlarms; i++){
     if((alarms[i].active && bitRead(alarms[i].days, now.dayOfTheWeek())) || snoozes[i] > 0){
       alarmMin = getDayMin(alarms[i].hrs, alarms[i].minutes);
       if(curMin == alarmMin || snoozes[i] == curMin){
         alarmState = true; 
         selected = false;
         lcd.cursor();
         curAlarm = i;
         break;
       }else if(curMin == (alarmMin - alarms[i].timeLight) && alarms[i].light){
         lightState = true;
       }
     }
   }
}

void onAlarmTick(){
  goalMoveCount += 1;
  goalMoveCount %= GOAL_MOVE_FREQ;
  if(goalMoveCount == 0){
    alarmGoalPos = changeVal(alarmGoalPos, 1, LCD_COLS + 1, LCD_COLS*LCD_ROWS -2);
  }
}

void setup() {
  lcd.begin(16, 2);
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(RED_PIN, OUTPUT);
  pinMode(LIGHT_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
  
  //load data from memory
  loadData();
  
  //rtc setup
  Wire.begin();
  rtc.begin();
  
  if(rtc.lostPower()){
    //print message to LCD
    lcd.print("RTC NOT Running");
    //sets rtc to date and time of sketch compile
    rtc.adjust(DateTime(__DATE__, __TIME__));
  }
  
  gotoTime();
}

void loop() {
  //get date/time
  now = rtc.now();
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
  
  if(alarmState){
    onAlarmTick();
  }
  
  if(now.minute() != oldMinute){
    checkAlarms();
    oldMinute = now.minute();
  }
  
  drawLCD();
   
  //delay fo rthe lcd to update
  delay(tickLen);
}
