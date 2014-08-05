#include "Arduino.h"
#include "MenuLib.h"
#include <LiquidCrystal.h>
#include <Encoder.h>

//Constants for defining strings as menuitem values
#define BACK -1
#define NO_VAL -2
#define ON -3

MenuItem::MenuItem(int value, String name){
	_value = value;
	_name = name;
}

int MenuItem::getVal(){
	return _value;
}

String MenuItem::getName(){
	return _name;
}

//functions for running menus (add to actual file
//since they rely on lcd object (unless that is added to the class))

//displays the cursor of the menu along with two items
//(the ones that are visible)
void displayItems(String selectedItem, String nextItem, 
					LiquidCrystal lcd){
	lcd.setCursor(0, 0);
	lcd.print(">");
	lcd.print(selectedItem);
	lcd.setCursor(1, 1);
	lcd.print(nextItem);
}

void updateEndArray(long encArray[], Encoder *enc){
	encArray[REAL_POS] = enc->read();
	newPos = (
}

//the position the encoder library sees (~360 ticks per rotation)
#define REAL_POS 0
//the new position of the encoder (24 ticks per rotation)
#define NEW_POS 1
//the old position of the encoder (24 ticks per rotation)
#define OLD_POS 2
 
//runs the menu selection
void runMenu(MenuItem items[], int numItems, 
				LiquidCrystal *lcd, Encoder *enc){
	long encInfo[3];

	while (!goBack){
		//update encoder 
		//FIXME should this be in another function?
		realPos = enc->read();
		newPos= (realPos + 1) / 4;	;  //calculate each increment based on
												   //the position. Add 1 so that division
												   //by 4 puts each increment in the middle
		//update index                             //of a division and not at the beginning
		if(newPos != oldPos){
			
		}
	}
}