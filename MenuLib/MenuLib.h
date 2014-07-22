#ifndef MenuLib_h
#define MenuLib_h

#include "Arduino.h"

class MenuItem {
		int _value;
		String _name;
	public:
		MenuItem(int value, String name);
		int getVal();
		String getName();
};

#endif