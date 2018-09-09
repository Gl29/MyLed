#include <LiquidCrystal_I2C.h> //Библиотека I2C для LCD
                               // LiquidCrystal()	создаёт переменную типа LiquidCrystal и принимает параметры подключения дисплея (номера выводов);
                               // begin()	инициализация LCD дисплея, задание параметров (кол-во строк и символов);
                               // clear()	очистка экрана и возврат курсора в начальную позицию;
                               // home()	возврат курсора в начальную позицию;
                               // setCursor()	установка курсора на заданную позицию;
                               // write()	выводит символ на ЖК экран;
                               // print()	выводит текст на ЖК экран;
                               // cursor()	показывает курсор, т.е. подчёркивание под местом следующего символа;
                               // noCursor()	прячет курсор;
                               // blink()	мигание курсора;
                               // noBlink()	отмена мигания;
                               // noDisplay()	выключение дисплея с сохранением всей отображаемой информации;
                               // display()	включение дисплея с сохранением всей отображаемой информации;
                               // scrollDisplayLeft()	прокрутка содержимого дисплея на 1 позицию влево;
                               // scrollDisplayRight()	прокрутка содержимого дисплея на 1 позицию вправо;
                               // autoscroll()	включение автопрокрутки;
                               // noAutoscroll()	выключение автопрокрутки;
                               // leftToRight()	задаёт направление текста слева направо;
                               // rightToLeft()	направление текста справа налево;
                               // createChar()	создаёт пользовательский символ для LCD-экрана.

class MyLCD : public LiquidCrystal_I2C
{
  public:
  

      MyLCD(int8_t lcd_Addr, int8_t lcd_cols, int8_t lcd_rows) :  LiquidCrystal_I2C( lcd_Addr,  lcd_cols,  lcd_rows) {};

    // void LCD_Print_temp(float t1, float t2) // Функция которая выводит на LCD значение температуры
    // {
    //   LiquidCrystal_I2C::clear();
    //   LiquidCrystal_I2C::setCursor(0, 0);   // Устанавливаем курсор в начало 1 строки
    //   LiquidCrystal_I2C::print("S1.temp="); // Выводим текст
    //   LiquidCrystal_I2C::print(t1);

    //   LiquidCrystal_I2C::setCursor(0, 1);   // Устанавливаем курсор в начало 1 строки
    //   LiquidCrystal_I2C::print("S2.temp="); // Выводим текст
    //   LiquidCrystal_I2C::print(t2);
    // }


    void LCD_Print(const char Str1[16], const float t1, const char Str2[16], const float t2) // Функция которая выводит на LCD значение температуры
    {   
        LiquidCrystal_I2C::backlight();
        LiquidCrystal_I2C::clear();
        LiquidCrystal_I2C::setCursor(0, 0); // Устанавливаем курсор в начало 1 строки

        LiquidCrystal_I2C::printstr(Str1);
        if (t1 != -99) {LiquidCrystal_I2C::print(t1);}
        
        LiquidCrystal_I2C::setCursor(0, 1);
        LiquidCrystal_I2C::printstr(Str2);
        if (t2 != -99) {LiquidCrystal_I2C::print(t2);}
    }

    // void  LCD_BackLightOff()
    // {
    //   LiquidCrystal_I2C::noBacklight();

    // }
};