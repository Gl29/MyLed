#include "Arduino.h"


struct stuct_LedBrightness              // сртуктура хранит параметры LED каналов
{
    String channelName;                 // текстовое имя LED канала 
    byte   channalNumber;               // номер  LED канала 
    uint16_t channalBrightness ;   // значение яркости канала  микросхема TLC5940NT - 16-канальный ШИМ драйвер с 12-битным регулированием скважности (0-4096) и 6-битным (0-63) регулированием тока.
};