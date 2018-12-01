#include "MyMENU.h"


// ------------------------------
// !!! Внимание, на досуге правильно переписать конструкторы с целью оптимизации кода, 
// посмотреть статью по поводу делегирования конструкторов
MyMENU::MyMENU()
{
 MyMENU::MenuAmount++; //увеличиваем счётчик созданных меню на 1 
};

MyMENU::MyMENU(t_MenuType MenuType, char *Row1, void(*on_click)(int8_t), int8_t func_param=1): _MenuType(MenuType), _ptr_Row1(Row1)
{ 
    MyMENU();
    ptr_on_click=on_click;
    _func_param= func_param;
}; 


MyMENU::MyMENU(t_MenuType MenuType, char *Row1, char *Row2): _MenuType(MenuType), _ptr_Row1(Row1), _ptr_Row2(Row2)
{
    MyMENU();
};


MyMENU::MyMENU(t_MenuType MenuType, char *Row1, const uint8_t *RowParamValue):_MenuType(MenuType), _ptr_Row1(Row1), _ptr_Row2_ParamValue(RowParamValue)
{
    MyMENU();
    UpdateRow2_Value();
    // char chrTMP[3];
    // sprintf(chrTMP, "%c%d", '>',*RowParamValue);
    // strcat(chrTMP, " \0");
    // strcpy(_Row2, chrTMP);
   // _ptr_Row2 = _Row2; 
};

MyMENU::MyMENU(t_MenuType MenuType, char *Row1, const uint8_t *RowParamValue, void(*on_click)(int8_t), int8_t func_param=1)
{
    MyMENU();
    _MenuType=MenuType; 
    _ptr_Row1=Row1;
    _ptr_Row2_ParamValue=RowParamValue;
     ptr_on_click=on_click;
    _func_param= func_param;   
    
    UpdateRow2_Value();

};

MyMENU::MyMENU(t_MenuType MenuType, char *Row1, char *Row2, void(*on_click)(int8_t),int8_t func_param=1):
         _MenuType(MenuType), _ptr_Row1(Row1), _ptr_Row2(Row2), ptr_on_click(on_click), _func_param(func_param)
{MyMENU();}
// ---------------------------------------------------------------------------------------------------------------------------------

void MyMENU::SetMenuType (t_MenuType MenuType)
// задаём тип меню
{ 
   _MenuType=MenuType;
}  

// void MyMENU::UpdateRow2_Value()
// {
//     char chrTMP[3];
//     sprintf(chrTMP, "%c%d", '>',*_ptr_Row2_ParamValue);
//     strcat(chrTMP, " \0");
//     strcpy(_Row2, chrTMP);
// }


void MyMENU::UpdateRow2_Value()
{
    char chrTMP_1[16]= {'\0'};  // в этот массив собираем итоговую строку
    char chrTMP[3];
    sprintf(chrTMP, "%c%d", '>',*_ptr_Row2_ParamValue);
    strcat(chrTMP_1, chrTMP);
    if (strlen(chrTMP_1)<16)
    {
         while (strlen(chrTMP_1)<16){strcat(chrTMP_1, " \0");} // "Добиваем" строчку  cпробелами
    } 
    strcpy(_Row2, chrTMP_1);
}





void MyMENU::UpdateRow (byte RowNumb,   char *Row1,  float *param1,  char *Row2,  float *param2)
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
         while (strlen(chrTMP_1)<16){strcat(chrTMP_1, " \0");} // "Добиваем" строчку  c пробелами
    }

    if(RowNumb ==1){strcpy(_ptr_Row1, chrTMP_1);}
    else {strcpy(_ptr_Row2, chrTMP_1);}
}


// void MyMENU::UpdateRow (byte RowNumb, const char *Row, const uint8_t *RowParamValue)
// // добавляем строку char[] RowNumb на экран (uint_8)
// {   
//     // Serial.print("*param1=");
//     // Serial.println(*param1);
//     char chrTMP[2];
//     //sprintf(chrTMP, "%d", *RowParamValue);
//     sprintf(chrTMP, "%c%d", '=',*RowParamValue);


//     char chrTMP_1[16]= {'\0'};  // в этот массив собираем итоговую строку
//     strcat(chrTMP_1, Row); 
//     //strcat(chrTMP_1, "="); 
//     strcat(chrTMP_1,  chrTMP);    

//     if (strlen(chrTMP_1)<16){
//         while (strlen(chrTMP_1)<16){strcat(chrTMP_1, " \0");} // "Добиваем" строчку  пробелами
//     }
//     if(RowNumb ==1) {strcpy(_ptr_Row1, chrTMP_1);}
//     else            {strcpy(_ptr_Row2, chrTMP_1);}
// }



void MyMENU::UpdateRow (byte RowNumb, const char *Row)
// добавляем строку char[] RowNumb на экран (uint_8)
{   
    char chrTMP_1[16]= {'\0'};  // в этот массив собираем итоговую строку
    strcat(chrTMP_1, Row); 
 
    if (strlen(chrTMP_1)<16){
        while (strlen(chrTMP_1)<16){strcat(chrTMP_1, " \0");} // "Добиваем" строчку  пробелами
    }
    if(RowNumb ==1) {strcpy(_ptr_Row1, chrTMP_1);}
    else            {strcpy(_ptr_Row2, chrTMP_1);}
}


char *MyMENU::GetRow (byte RowNumber) //char* GetRow (byte RowNumber)
// возвращаем указанную строку (массив char)
{ 
    if (RowNumber==1){return _ptr_Row1;}
    else {return _ptr_Row2;}
}
    
MyMENU::t_MenuType MyMENU::GetMenuType ()
// возвращаем тип меню
{ 
    return _MenuType;
}    

