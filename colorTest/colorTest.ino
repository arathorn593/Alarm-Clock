#include <LiquidCrystal.h>

#define BLUE_PIN 9
#define GREEN_PIN 10
#define RED_PIN 11

LiquidCrystal lcd(1, 4, 5, 6, 7, 8)

void setup(){
  //LCD setup
  //set up the LCD's col and rows
  lcd.begin(16, 2);
  
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(RED_PIN, OUTPUT);
}


