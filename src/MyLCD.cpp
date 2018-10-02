#include "MyLCD.h"
#include <LiquidCrystal_I2C.h>



// class MyLCD : public LiquidCrystal_I2C
// {
//   public:
//     String LCDRow1; // перемення в которой записываем верхнюю (1) строку LCD дисплея
//     String LCDRow2; // перемення в которой записываем нижнюю (2) строку LCD дисплея


     MyLCD::MyLCD(uint8_t lcd_Addr,uint8_t lcd_cols,uint8_t lcd_rows) : LiquidCrystal_I2C ( lcd_Addr, lcd_cols, lcd_rows)   // MyLCD передается в конструктор с параметром класса FirstClass
     {}
 
    void MyLCD::LCD_Print() // Функция которая выводит на LCD значение температуры
    {   
        LiquidCrystal_I2C::setCursor(0, 0); // Устанавливаем курсор в начало 1 строки
        LiquidCrystal_I2C::print(LCDRow1);
        LiquidCrystal_I2C::setCursor(0, 1);
        LiquidCrystal_I2C::print(LCDRow2);
     
      #ifdef DEBUG_LCDTest
        Serial.print ("LCDRow1=");
        Serial.println (LCDRow1);
        Serial.print ("LCDRow2=");
        Serial.println (LCDRow2);
      #endif
    }

	

// };