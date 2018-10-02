#include "Menu.h"




// Меню, первый пункт это не параметр, а просто выход с сохранением
	//    Menu::_MenuItem	Menu::MenuItems[] = {
	// 		{ "SAVE & EXIT     ", MenuSaveRam,		Null,		"      Press [+]" },
	// 		{ "D01: Date Day   ", SettingDateDay,	DateDay,	"VAL: %02d" },
	// 		{ "D02: Date Mount ", SettingDateMonth, DateMonth,	"VAL: %02d" },
	// 		{ "D03: Date Year  ", SettingDateYear,	DateYear,	"VAL: 20%02d" },
	// 		{ "D04: Time Hour  ", SettingTimeHour,	TimeHour,	"VAL: %02d" },
	// 		{ "D05: Time Min   ", SettingTimeMin,	TimeMin,	"VAL: %02d" }
	// 	};

	//   Menu::MenuItems[0] = {"SAVE & EXIT     ", Menu::MenuSaveRam, Menu::Null, "      Press [+]" };
		


// Функции вызываемые для изменения значений переменных

void Menu::MenuSaveRam(int Concat)
{ // SAVE & EXIT
	if (Concat == 1)
	{					   //TODO Сохранение параметров в RAM };
		MenuEnter = false; // Выход из меню
	}
}

void Menu::SettingDateDay(int Concat)
{
	DateDay = constrain(DateDay + Concat, 1, 31); // Изменяем с ограничением
}

void Menu::SettingDateMonth(int Concat)
{
	DateMonth = constrain(DateMonth + Concat, 1, 12);
}

void Menu::SettingDateYear(int Concat)
{
	DateYear = constrain(DateYear + Concat, 0, 99);
}

void Menu::SettingTimeHour(int Concat)
{
	TimeHour = constrain(TimeHour + Concat, 0, 23);
}

void Menu::SettingTimeMin(int Concat)
{
	TimeMin = constrain(TimeMin + Concat, 0, 59);
}
