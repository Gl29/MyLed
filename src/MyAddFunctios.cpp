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


