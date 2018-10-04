#include "Arduino.h"
#include <TimeLib.h>

// структура для хранение / ведения кодов возникающих ошибок
struct Err {
    String  TextErr;
    uint8_t codeErr;
};

// структура для хранение текуих времни и даты
struct CurrDateTimeSTR {
    String Date;
    String Time;
    char CharDate[11]; // храним дату в виде массива char
    char CharTime[6];  // храним время в виде массива char
    char* ptrCharDate=CharDate; // перемення указатель на массив char
    char* ptrCharTime=CharTime;
    
};

String printDigits(int);
void GetSTR_DateTime (tmElements_t &t, CurrDateTimeSTR &CDT);