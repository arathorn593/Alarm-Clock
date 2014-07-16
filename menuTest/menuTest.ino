#include <LiquidCrystal.h>

#include <Encoder.h>
#include <LiquidCrystal.h>

#define BLUE_PIN 9
#define GREEN_PIN 10
#define RED_PIN 11

#define MENU_SIZE 3
char * menuItems[] = {
  "Clock",
  "Alarms",
  "Settings"
};
int index;

LiquidCrystal lcd(1, 4, 5, 6, 7, 8);
Encoder enc(2,3);

void displayMenu(char** items){
    int numItems = sizeof(items);
    Serial.println(numItems);
}

void setup(){
  Serial.begin(9600);
  lcd.begin(16, 2);
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(RED_PIN, OUTPUT);
  
  index = 0;
}

long oldPosition = 0;
long newPosition = 0;
long realPosition= 0;
void loop(){
  displayMenu(menuItems);
  //update encoder
  realPosition = enc.read();
  newPosition = (realPosition + 1) / 4;  //calculate each increment based on
                                         //the position. Add 1 so that division
                                         //by 4 puts each increment in the middle
  //update index                         //of a division and not at the beginning
  if(newPosition != oldPosition){
    //wrap around index if it is out of bounds
    if(newPosition > oldPosition){
      if(index >= MENU_SIZE -1){
        index = 0;
      }else{
        index++;
      }
    }else{
      if(index <= 0){
        index = MENU_SIZE -1;
      }else{
        index--;
      } 
    }
    oldPosition = newPosition;
  }    
  
  //clear lcd
  lcd.clear();
  
  //print pointer
  lcd.setCursor(0, 0);
  lcd.print(">");
  
  //print first line
  lcd.print(menuItems[index]);
  
  //print second line
  lcd.setCursor(1, 1);
  
  //if the next index is out of bounds (aka the last item) then print the first item
  if(index >= MENU_SIZE - 1){
    lcd.print(menuItems[0]); 
  }else{
    lcd.print(menuItems[index + 1]);
  }
  
  //delay for lcd to update
  delay(50);
}
