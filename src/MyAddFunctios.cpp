#include "MyAddFunctions.h"

String printDigits(int digits) { // функция превращает число в текстовую строку, если число из одной цифры то дописывает ведущий ноль
  String t;
  if(digits < 10) 
    {t="0"+ String(digits);}
    else {t=String(digits);}
    return t;
}

// // работает но не нужна
// void GetSTR_DateTime (tmElements_t &t, CurrDateTimeSTR &CDT) // Возвращаем дату или время в виде текстовой строки
// { 
//    CDT.Time = printDigits(t.Hour) + ":" + printDigits(t.Minute);// + "\0";
//    CDT.Date = printDigits(t.Day) + "/" + printDigits(t.Month)  + "/" +  tmYearToCalendar(t.Year);// + "\0";     
//    CDT.Date.toCharArray(CDT.CharDate,11); 
//    CDT.Time.toCharArray(CDT.CharTime,6);  

// }

//
// Функция определяет количество дней, прошедших с Рождества Христова
// до указанного дня. Параметры: d (1-31), m (1-12), y (1-...)
// На самом деле она считает так, как будто всё это время
// действовал григорианский каледарь, но для большинства
// расчётов это не важно
//
// ПРИМЕРЫ:
// 1. Определить количество дней между датами 02.04.2018 и 01.02.2018
//      dayFromXmass(2,4,2018) - dayFromXmass(1,2,2018)
//      Результат - 60
// 2. Определить день недели для даты 02.04.2018
//      dayFromXmass(2,4,2018) % 7 // 0-понедельник - 6-воскресенье
//      Результат - 0 (понедельник)
//
uint32_t dayFromXmass(const int day,const int month,const int year) 
{
  const int daysOfYear[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
  const int y1 = year-1;
  return 365L*y1+y1/4-y1/100+y1/400+daysOfYear[month-1]+day-1+((!(year&3))&&((year%100)||(!(year%400))))*(month > 2);
}




