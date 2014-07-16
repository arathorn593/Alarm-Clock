using namespace std

class MenuItem {
	public:
		MenuItem(int link, string itemName);	//-1 if no link is wanted
		int getLinkIndex();
		bool isLinked();
		string getName();
	protected:
		bool linked;
		int linkIndex;
		string name;
		
}

class Menu {
	public:
		
	private:
		int numItems;
		void Menu(char** items, int lenght);
		void getLength(){return numItems};

		
}