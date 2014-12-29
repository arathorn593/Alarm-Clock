#include <LiquidCrystal.h>
#include <Encoder.h>

//Pin definitions
#define BLUE_PIN 9
#define GREEN_PIN 10
#define RED_PIN 11

//mode constants
#define TIME_MODE 0
#define MAIN_MENU 1

//Global variables
//mode/menu that is displayed currently
int mode = TIME_MODE;

String displayText = "hello";


//LCD/Encoder setup
Encoder enc(2,3);
LiquidCrystal lcd(1, 4, 5, 6, 7, 8);

void setup() {
  lcd.begin(16, 2);
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(RED_PIN, OUTPUT);
}

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

void onEncIncrement(){
  displayText = "Increment";
}

void onEncDecrement(){
  displayText = "Decrement";
}

void onEncStill(){
  return;
}


void onTick(){
  displayText = "tick";
}

void drawLCD(){
  //clear lcd
  lcd.clear();
  switch(mode){
     case TIME_MODE:
       break;
       
     case MAIN_MENU:
       break;
  }
}

void loop() {
  onTick();
  //update encoder and take appropriate action
  int encChange = getEncShift();
  if(encChange > 0){
    onEncIncrement();
  }else if(encChange < 0){
    onEncDecrement();
  }else{
    onEncStill();
  }
  
  drawLCD();
    
  //delay fo rthe lcd to update
  delay(50);
}
