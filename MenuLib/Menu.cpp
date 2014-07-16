//Menu item functions
/*link = the index of the menu array that is below this item
 *itemName = the name of the item in the menu (max 11 char)
 */
#include "Menu.h"
 
MenuItem::MenuItem(int link, string itemName){
	if(link < 0){
		linked = false;
	}else{
		linkIndex = link;
		linked = true;
	}
	name = itemName;
}

int MenuItem::getLinkIndex(){
	return linkIndex;
}

bool MenuItem::isLinked(){
	return linked;
}

string MenuItem::getName(){
	return name;
}

