#include "MyAddFunctions.h"

String printDigits(int digits) { // функция превращает число в текстовую строку, если число из одной цифры то дописывает ведущий ноль
  String t;
  if(digits < 10) 
    {t="0"+ String(digits);}
    else {t=digits;}
    return t;
}

String GetSTR_DateTime (tmElements_t &t,char Date_Time) // Возвращаем дату или время в виде текстовой строки
{ String tSTR;
    switch (Date_Time) {
        case 'T':
            tSTR= printDigits(t.Hour) + ":" + printDigits(t.Minute) + "\0";
            break;
        case 'D':
            tSTR = printDigits(t.Day) + "/" + printDigits(t.Month)  + "/" +  tmYearToCalendar(t.Year) + "\0";    
        break;
            }   

return tSTR;
}