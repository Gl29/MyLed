#ifndef MYMENU_H
#define MYMENU_H

#include "Arduino.h"
#include <string.h>



class MyMENU 
//  Класс содержит одно экранное меню, т.е 2 строки для LCD дисплея.
//  для работы с LCD дисплеем просто создаём массив из переменных данного класса.
{

public:
        enum t_MenuType {MainMenu,LEDMenu,ParamMenu, N_A};      // Main         - Основной экран   =0
                                                                // LEDMenu      - меню LED экранов =1
                                                                // Param        - меню параметров  =2
            

        enum t_MenuOperationMode {NotEdit,RowSelect,RowEdit};   // NotEdit      - Не редактируем  =0
                                                                // RowSelect    - Режим выбора строки =1
                                                                // RowEdit      - Режим редактированя строки  =2
    





// MyMENU(t_MenuType MenuType, char *Row1, char *Row2,  void(*on_click)(int8_t), int8_t func_param=1)
// {
//    // _Menu.LCDNumber=0;
//    _MenuType =MenuType;
// //    strcpy(_Row1, Row1);    
// //    strcpy(_Row2, Row2);    
//    _Row1=Row1;
//    _Row2=Row2;
   
//    _on_click = on_click;    
//    _func_param = func_param; 

// }



MyMENU(t_MenuType MenuType, char *Row1, char *Row2, void(*on_click)(int8_t),int8_t func_param=1):
         _MenuType(MenuType), _Row1(Row1), _Row2(Row2), _on_click(on_click), _func_param(func_param)
{
//    // _Menu.LCDNumber=0;
//    _MenuType =MenuType;
// //    strcpy(_Row1, Row1);    
// //    strcpy(_Row2, Row2);    
//    _Row1=Row1;
//    _Row2=Row2;
   

}



void DebugPrint()
{
    Serial.print(_MenuType);   Serial.print(", "); 
    Serial.print(_Row1);       Serial.print(", "); 
    Serial.print(_Row2);       Serial.print(", "); 

    Serial.println(_func_param);

}

void UpdateRow (byte RowNumb,   char *Row1,  float *param1,  char *Row2,  float *param2)
// добавляем 2 строки char[] RowNumb на экран (Float)
{   
    char chrTMP_1[16]= {'\0'};  // в этот массив собираем итоговую строку
    char chrTMP[4];             //этот для преобразрвания float в char

    dtostrf(*param1, 4, 1, chrTMP); // преобразуем  param1 (float) в char
    strcat(chrTMP_1, Row1); 
    strcat(chrTMP_1, chrTMP);    

    dtostrf(*param2, 4, 1, chrTMP); // преобразуем  param1 в char
    strcat(chrTMP_1, Row2);   
    strcat(chrTMP_1, chrTMP); 

    if (strlen(chrTMP_1)<16){
         while (strlen(chrTMP_1)<16){strcat(chrTMP_1, " \0");} // "Добиваем" строчку  cпробелами
    }

    if(RowNumb ==1){strcpy(_Row1, chrTMP_1);}
    else {strcpy(_Row2, chrTMP_1);}

 

}


// void UpdateRow (byte RowNumb, const char *Row, const uint8_t *RowParamValue)
// // добавляем строку char[] RowNumb на экран (uint_8)
// {   
//     // Serial.print("*param1=");
//     // Serial.println(*param1);
//     char chrTMP[2];
//     sprintf(chrTMP, "%d", *RowParamValue);

//     char chrTMP_1[16]= {'\0'};  // в этот массив собираем итоговую строку
//     strcat(chrTMP_1, Row); 
//     strcat(chrTMP_1, "="); 
//     strcat(chrTMP_1,  chrTMP);    

//     if (strlen(chrTMP_1)<16){
//         while (strlen(chrTMP_1)<16){strcat(chrTMP_1, " \0");} // "Добиваем" строчку  пробелами
//     }
//     if(RowNumb ==1) {strcpy(_Row1, chrTMP_1);}
//     else            {strcpy(_Row2, chrTMP_1);}
// }




void UpdateRow (byte RowNumb, const char *Row)
// добавляем строку char[] RowNumb на экран (uint_8)
{   
    Serial.println();
    Serial.println("------Func UpdateRow____Start");
    Serial.print("*RowNumb=");
    Serial.println(RowNumb);

    Serial.print("*Row=");
    Serial.println(*Row);

    Serial.print("Row=");
    Serial.println(Row);

    Serial.print("_Row1=");
    Serial.println(_Row1);
    Serial.print("_Row2=");
    Serial.println(_Row2);



    char chrTMP_1[16]= {'\0'};  // в этот массив собираем итоговую строку
    Serial.print("chrTMP_1[16]=");
    Serial.println(chrTMP_1);   
    strcat(chrTMP_1, Row); 

    Serial.print("strcat(chrTMP_1, Row)=");
    Serial.println(chrTMP_1);   
    

   
    if (strlen(chrTMP_1)<16){
        while (strlen(chrTMP_1)<16){strcat(chrTMP_1, " \0");} // "Добиваем" строчку  пробелами
    }

    Serial.print("strlen(chrTMP_1)<16...=");
    Serial.println(chrTMP_1);  

    if(RowNumb ==1) {strcpy(_Row1, chrTMP_1);}
    else            {strcpy(_Row2, chrTMP_1);}

    Serial.println();
    Serial.println(chrTMP_1);
    Serial.println();
    Serial.println("------Func UpdateRow____End");
    Serial.println();
    
}







char *GetRow (byte RowNumber) //char* GetRow (byte RowNumber)
// возвращаем указанную строку (массив char)
{ 
    if (RowNumber==1){return _Row1;}
    else {return _Row2;}
}
    
t_MenuType GetMenuType ()
// возвращаем тип меню
{ 
    return _MenuType;
}    

void SetMenuType (t_MenuType _MenuType)
// задаём тип меню
{ 
   _MenuType= _MenuType;
}  

private:
        t_MenuType       _MenuType;
        char            *_Row1;
        char            *_Row2;
	    void      (*_on_click)(int8_t);	    // Ссылка на функцию вызова обрабоки изменения значения, параметр +1 или -1)
	    int8_t           _func_param ;			    // Указатель на переменную значения
};





#endif