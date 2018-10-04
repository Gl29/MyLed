#include "Arduino.h"
#include <LiquidCrystal_I2C.h>


//____________________________________
// Не используется
// ____________________________________


class MyLCD : public LiquidCrystal_I2C
{
  public:
    String LCDRow1; // перемення в которой записываем верхнюю (1) строку LCD дисплея
    String LCDRow2; // перемення в которой записываем нижнюю (2) строку LCD дисплея
	bool NeedOff = false; // признак того что подсветку можно отключить

    MyLCD(uint8_t lcd_Addr,uint8_t lcd_cols,uint8_t lcd_rows) ;   // MyLCD передается в конструктор с параметром класса FirstClass
  
    void LCD_Print(); // Функция которая выводит на LCD значение температуры
  
};