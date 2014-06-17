#include <EEPROM.h>

/* Light Alarm Clock */

#define MEM_SIZE 1024
#define ALARM_NUM  100 //number of alarms
#define ALARM_START 5 //byte index where the first alarm is

#define RED_LCD_INDEX 0
#define GREEN_LCD_INDEX 1
#define BLUE_LCD_INDEX 2
#define DATE_DISP_INDEX 3
#define TIME_TYPE_INDEX 4

byte redLCD;
byte greenLCD; 
byte blueLCD;
boolean displayDate;
boolean militaryTime; //if false then 12 hour AM/PM

typedef struct{
  byte settings; //yes/no for each alarm. order = alarm?, light?, sound?
  byte days; //first bit = nothing, days procede from there
  byte hour; //decimal hour(24)
  byte minutes; //decimal
  byte timeLight; //decimal, time in minutes before alarm
}Alarm;

/*HOW AN ALARM IS STORED IN MEMORY
  5 bytes: settings, days, hour, minutes, time before light
  setting byte signals there is an alarm: if first bit 
  (position 1, index 0, furthest right) is 1 if there is an alarm
*/

void setup(){
  //input general settings
  /*byte(index) function
             0  red LCD
             1  green LCD
             2  blue LCD
             3  display date (1/0) (in byte index 0)
             4  military(1)/norm(0) (in byte index 0)
             5  alarms start
  */
  redLCD = EEPROM.read(RED_LCD_INDEX);
  greenLCD = EEPROM.read(GREEN_LCD_INDEX);
  blueLCD = EEPROM.read(BLUE_LCD_INDEX);
  
  if(bitRead(EEPROM.read(DATE_DISP_INDEX), 0) == 1){
    displayDate = true;
  }else{
    displayDate = false;
  }
  
  if(bitRead(EEPROM.read(TIME_TYPE_INDEX), 0) == 1){
    militaryTime = true;
  }else{
    militaryTime = false;
  }
  
  //input alarms.
   int value;
   for(i = alarmStart; i < memorySize; i +=4 ){
     value = EEPROM.read(i);
     if(bitRead(value, 0) == 1){
       
     }
   }
}

