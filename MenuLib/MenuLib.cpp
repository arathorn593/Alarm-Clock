#include "Arduino.h"
#include "MenuLib.h"

//Constants for defining strings as menuitem values
#define NO_VAL -1
#define ON -2

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

