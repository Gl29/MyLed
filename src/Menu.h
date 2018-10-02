#ifndef MENU_H
#define MENU_H

#include "Arduino.h"

class Menu{
	//private:
public:
		// Это переменные для работы меню
		int	MenuCurent = 0; 		// Выбранный пункт меню
		bool MenuEnter = false; 	// Мы находимся в меню?
		int	MenuCount = 6; 			// Количество пунктов меню


		// Это все параметры которые можем менять в меню
		int Null = 0; // Этот параметр как заглушка, если менять нечего, костыль конечно ...
		int DateDay = 1;
		int DateMonth = 1;
		int DateYear = 18;
		int TimeHour = 0;
		int TimeMin = 0;
		int TimeSecond = 0;



		// Тип структуры данных описывающих пункт меню
		typedef struct _MenuItem {
			String title;			// Имя меню
			void(*on_click)(int);	// Ссылка на функцию вызова обрабоки изменения значения, параметр +1 или -1)
			int &param;				// Ссылка на переменную значения
			char format[16];		// Формат вывода значения переменной
		} ;

		 _MenuItem MenuItems[6];
		


//	public:
	Menu();

	// Функции вызываемые для изменения значений переменных
	void MenuSaveRam(int );
	void SettingDateDay(int); // Изменяем с ограничением
	void SettingDateMonth(int);
	void SettingDateYear(int);
	void SettingTimeHour(int);
	void SettingTimeMin(int);

};





#endif