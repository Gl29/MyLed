#include "Arduino.h"
//#include <TimeLib.h>

// // пока отключил за ненадобностью на данном этапе разработки
// //// структура для хранение / ведения кодов возникающих ошибок
// struct Err {
//     String  TextErr;
//     uint8_t codeErr;
// };

// // работает но ненужна
// //// структура для хранение текуих времни и даты
// struct CurrDateTimeSTR {
//     String Date;
//     String Time;
//     char CharDate[11]; // храним дату в виде массива char
//     char CharTime[6];  // храним время в виде массива char
//     char* ptrCharDate=CharDate; // перемення указатель на массив char
//     char* ptrCharTime=CharTime;
    
// };

String printDigits(int);
//void GetSTR_DateTime (tmElements_t &t, CurrDateTimeSTR &CDT);

// char * FloatToChar(float &_num, byte &len)
// { 
    
//     return dtostrf(f_val,7, 3, outstr);

// }



// Функция определяет количество дней, прошедших с Рождества Христова
// до указанного дня. 
uint32_t dayFromXmass(const int day,const int month,const int year); 