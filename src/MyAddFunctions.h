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
};

String printDigits(int);
void GetSTR_DateTime (tmElements_t &t, CurrDateTimeSTR &CDT);