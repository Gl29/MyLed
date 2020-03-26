#ifndef MYMENU_H
#define MYMENU_H

#include "Arduino.h"
#include <string.h>
//#define DEBUG_MyMenu


class MyMENU 
//  Класс содержит одно экранное меню, т.е 2 строки для LCD дисплея.
//  для работы с LCD дисплеем просто создаём массив из переменных данного класса.
{

public:
        enum t_MenuType {MainMenu,LEDMenu,ParamMenu, TestLedChnlBright, N_A};      // Main         - Основной экран   =0
                                                                // LEDMenu      - меню LED экранов =1
                                                                // Param        - меню параметров  =2
                                                                // TestLedChnlBright -- меню тестирования яркости каналов. Функция LedChnlBrightSet
            
        enum t_MenuOperationMode {NotEdit,RowEdit};             // NotEdit      - Не редактируем  =0
                                                                // RowSelect    - Режим выбора строки =1
                                                                // RowEdit      - Режим редактированя строки  =2
        static byte MenuAmount;                                 // статическая переменнуая-член класса, счетчик обьектов класса
        void (*ptr_on_click)(int8_t) = nullptr;	                // Указатель на функцию вызова обрабоки изменения значения, параметр +1 или -1)




MyMENU();
MyMENU(t_MenuType , char* , void(* on_click)(int8_t), int8_t);
MyMENU(t_MenuType , char* , char*);
MyMENU(t_MenuType , char* , const uint8_t*);
MyMENU(t_MenuType , char* , const uint8_t*, void(* on_click)(int8_t), int8_t);
MyMENU(t_MenuType , char* , char*, void(*on_click)(int8_t), int8_t);



#ifdef DEBUG_MyMenu
    void DebugPrint()
    {
        char str[80];
        
        Serial.print(_MenuType);       Serial.print(", "); 
        Serial.print(_ptr_Row1);       Serial.print(", "); 
        Serial.print(_ptr_Row2);       Serial.print(", "); 
        
        sprintf (str, "%p", ptr_on_click); // печатаем адрес функции  
        Serial.print ("sprintf: ptr_on_click: ");
        Serial.print(str); Serial.print(", ");
        Serial.println(_func_param);

    }
#endif

void SetMenuType (t_MenuType);

void UpdateRow2_Value();                 //функция берет значение из указателя на внешний параметр (_ptr_Row2_ParamValue) и превращает его в строку экрана _Row2
void UpdateRow (byte, char *,  float *,  char *,  float *);
void UpdateRow (byte, const char *);
void UpdateRow (byte, const char *, const uint8_t *);

char *GetRow (byte);
t_MenuType GetMenuType ();


private:
        t_MenuType       _MenuType;
        char             _Row2[17];                 // вторую строку экрана храним в переменных класса
        char            *_ptr_Row1=nullptr;         // указатель на 1ю строку меню меню экземпляра данного класса
        char            *_ptr_Row2=_Row2;           // указатель на 2ю строку меню экземпляра данного класса 
        const uint8_t   *_ptr_Row2_ParamValue;      // указатель на переменную / значение параметра хранится вне каласса (например яркость экрана или время итд)
	          int8_t     _func_param ;		        // Переменную значения которое передатся в функцию  void (*ptr_on_click)(int8_t)
};
#endif