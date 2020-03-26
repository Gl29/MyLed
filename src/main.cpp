//------- Библиотеки -------------------------------------------------------------------------------------
#include <Arduino.h>

// #include "MemoryExplorer.h"     // Позволяет отследить использование памяти

#include <OneWire.h>            //Библиотека 1-Wire. для цифровых датчиков температуры 
#include <EEPROM.h>          // Библиотека для работы с EEPROM 
#include "Tlc5940.h"            // библиотека для работы с драйвером светодиодов tlc5940
//#include <Wire.h>
//#include <RTClib.h>
// #include <TimeLib.h>
// #include <DS1307RTC.h>       //a basic DS1307 library that returns time as a time_t. 
//#include <Time.h>

#include <LiquidCrystal_I2C.h>  //Библиотека I2C для LCD

#include <iarduino_RTC.h>       // подключаем библиотеку для работы с RTC модулем https://lesson.iarduino.ru/page/urok-17-podklyuchenie-rtc-chasy-realnogo-vremeni-s-knopkami/
                                // http://iarduino.ru/file/235.html


// #include <leOS.h>            // Шедуллер задач -- если запущено несколько задач начинаются тормоза, пробуем без него
//#include <LiquidMenu.h>         // Библиотека для работы с меню -- каждый экран "жрёт" много памяти

//----- Самописныйе блоки кода, дабы не писать всё в Main ---------------------------------------------------
#include "MyMenu.h"              // вспомогательные функции для вывода на экран 
#include "Buttons.h"            // работа с кнопками
#include "MyAddFunctions.h"     // сборник дополнительных функций вынесен в отдельный файл "дабы не засорять эфир" 
#include "MyDC18B20.h"          // код для работы с датчикками темперетуры
#include "LedChannel.h"         // код для работы с LED каналами


#define DEBUG_Setup              // Включаем общий режим отладки
// #define DEBUG_DS18B20           // Включаем режим отладки Датчика температуры
//#define DEBUG_Loop             // Включаем режим отладки для функции Void LOOP
#define DEBUG_DemoMode           // Включает режим отладки режима DemoMode


#define SetSystemTime            // Включает режим установки системного времени
//  #define Debug_SetSystemTime     // Включает режим отладки в момент установки системного времени
#define DEBUG_Func_buttons_control
#define DEBUG_MyMenu

//-------- Стартовые параметры -----------------------------------------------------------------------
#define SERIAL_BAUD 115200          //115200 //9600 //57600 //
                                            //#define INTERVAL2  5*60*1000UL
#define TimeInterval_DS18D20        10*1000 //15000 //10000 // Время опроса датчиков температуры м.сек
#define TimeInterval_RTC            1000    // периодичность обновления информации о текущем времени м.сек
//#define TimeInterval_LCDUpdate      1000  // Периодичность обновления экрана м.сек / меню
#define TimeInterval_KeyRead        1// 500     // Интервал опроса кнопок м.сек

#define TimeInterval_LedCNL_Test    100    // Время через которое добавляется +1 условная минтута времени для процедуры теста определения яркости ЛЕД канала
#define TimeInterval_LCDBacklight   10*1000 // Время через которое выключается подсветка экрана м.сек
#define DC18_MaxGoodTemp            30      // Максимальная температура после превышения которой срабатывает событие (градусы цельсия)
//#define numbLEDChannel              5       // количество каналов LED
//#define numMaxScreeNumb             4       // количество экранов меню



//-------- PINs к которым прицеплена переферия --------------------------------------------------------
#define portOneWire 2                   // указаываем порт OneWire   протокол 1-Wire
#define portDC18B20PWM_1 5              // указаываем порт для PWM DC18B20_1  
#define portDC18B20PWM_2 6              // указаываем порт для PWM DC18B20_2
#define portButtonsRead A0              // указаываем порт на котором будем слушать нажатие кнопок


//static Tlc5940 Tlc;
static OneWire DS18_OneWare(portOneWire);          // переменная OneWire протокола 1-Wire
static struct_DS18_setting DS18_settings;          // структра для хранения набора параметров для датчиков температуры 
static iarduino_RTC time(RTC_DS1307);              // объявляем переменную time для работы с библиотекой, указывая название модуля RTC_DS1307     
static Button MyButtons = Button(portButtonsRead); // Обьект для отработки нажатия кнопок
static LiquidCrystal_I2C lcd(0x27, 16, 2);         // объявляем  переменную lcd для работы с LCD дисплеем, указывая параметры дисплея (адрес I2C = 0x27, количество столбцов = 16, количество строк = 2)

static uint32_t TimerPrevMillis[5];                // Массив для хранения времени прошедшего с момента последнего срабатывания таймеров
                                            // Таймеры:
                                            // 0 - Считывание времени из RTC+ Обновление даты/времени на экране
                                            // 1 - Опрос датчиков температуры
                                            // 2 - Включение подсветки                    
                                            // 3 - Счётчик времени для обновления  яркости ЛЕД каналов
                                            // 4 - 
                        

static uint8_t activeScreenNumber   = 0;        // Номер активного экрана: 
                                            // 0   - Время/дата //Темперетура 
                                            // 1   - Яркость LED линии 1
                                            // 2   - Яркость LED линии 2
                                            // ........


// static uint8_t dateTimeSetMode        = 1;        // Переменная хранит режим установки времени: 
//                                             // 0-нет 1-нет (секунды сбрасываем автоматом) 
//                                             // 2-мин 3-час 4-день 5-мес 6-год 7-день_недели //1-сек
//                                             // 8-11 - 

static uint8_t EPROM_NeedWrite         =0;         // признак того что необходимо провести запись парамтеров в EPROM
static uint8_t demoMode                =0;         // переменная-флаг: обычный режим=0 , демо режим =1 (тест лед каналов)  
static uint8_t timerOnOff              =1;         // переменная-флаг: таймеры включены=1, таймеры выключены =0  

// uint8_t tmpHour                 =0;         // Храним текущее значение "часа" для демо режима 
// uint8_t tmpMinute               =0;         // Храним текущее значение "минут" для демо режима


 //MyMENU  Menu[numbLEDChannel+1];
 MyMENU::t_MenuOperationMode CurrOperationMode=MyMENU::NotEdit;


static LedChannel LedSettings [5] =                                 // структура для зранетия параметров LED каналов
        {
            {"ColdWhite_1     ", 20},  // название, яркость в %%
            {"ColdWhite_2     ", 30},
            {"WarmWhite_1     ", 30},
            {"Red             ", 30},
            {"Blue            ", 30}
        };


// блок функций для корректироки значений параметров меню
// Написать: Функция корректировки текущего времени/даты с помощью кнопок
void setDateTime(int8_t i){
     //tmpI++;
     };


// Задаём значение целовой (максимальной) яркости канала
void setTargetChannelBrightness(int8_t increment)
    {
            // !!!!!! activeScreenNumber-1 будет работать только при условии что параметры лед каналов будут начинаться со второго экрана
            // по хорошему в функцию также надо перадавать номер канала по кторому мы хотим поменять яркость
         //   LedSettings[activeScreenNumber-1].targetChnlBright_percent += increment;};
        switch (increment)
        {
            case -1:
                LedSettings[activeScreenNumber-1].BrightnessDOWN();
                break;
            case 1:
               LedSettings[activeScreenNumber-1].BrightnessUP();
                break;
        }    
            
    }      
              



//  Написать: функция корректировка значений доп.параметров
void setParam(int8_t i){
     //tmpI++;
     };



// шаблоны функций
void EPROM_Write(int8_t);
void EPROM_Read ();
void LedChnlBright_TestOnOFF(int8_t);
void TimerOnOff(int8_t);


void LCD_Update();

#define TmpChr "LedBrt. test 0/1"
char mnuLedBrtTest[17]=TmpChr;//"LedBrt. test 0/1";
char chrDateTime[17]     ="________________";   // переменная необходима для хранения первой строки меню (в классе только указатель на эту строку)  

//  byte MyMENU::MenuAmount=0;                      // инициация счётчика количества членов класса MyMENU
static MyMENU Menu[] = {                                           // массив для хранения строк меню, каждая строка - 1 экран
    { MyMENU::MainMenu, chrDateTime, setDateTime,1},
    { MyMENU::LEDMenu,  LedSettings[0].channelName, LedSettings[0].ptr_channelBrightness, setTargetChannelBrightness, 1}, 
    { MyMENU::LEDMenu,  LedSettings[1].channelName, LedSettings[1].ptr_channelBrightness, setTargetChannelBrightness, 1},   
    { MyMENU::LEDMenu,  LedSettings[2].channelName, LedSettings[2].ptr_channelBrightness, setTargetChannelBrightness, 1},    
    { MyMENU::LEDMenu,  LedSettings[3].channelName, LedSettings[3].ptr_channelBrightness, setTargetChannelBrightness, 1},
    { MyMENU::LEDMenu,  LedSettings[4].channelName, LedSettings[4].ptr_channelBrightness, setTargetChannelBrightness, 1},        
    { MyMENU::ParamMenu,    "Param_1         ",  "___",             setParam,                   1},   
    { MyMENU::ParamMenu,    "Save (N=0 / Y=1)",  &EPROM_NeedWrite,  EPROM_Write,                1}, 
    { MyMENU::ParamMenu,    "Timer:ON=1/OFF=0",  &timerOnOff,       TimerOnOff,                 1},
    { MyMENU::TestLedChnlBright,    mnuLedBrtTest,  &demoMode,      LedChnlBright_TestOnOFF,    1}, 
};

// 
void TimerOnOff(int8_t increment)
{
    switch (increment)
    {
        case -1:
            timerOnOff = timerOnOff+increment==0?0:1;
            break;
        case 1:
            timerOnOff = timerOnOff+increment==1?1:0;
            break;
    }
}



void EPROM_Write(int8_t increment)
{   

    EPROM_NeedWrite +=increment;

    if (EPROM_NeedWrite==1)
    {
        EEPROM.put(0, 111); // пишем в нулевой регистр 111 как признак того что произедена запись имеено из данной функции и последующие ячейки содержат параметры а не мусор
             for (int i=0;i<5;i++)
        {
            EEPROM.put(i+1, LedSettings[i].targetChnlBright_percent);
        }
            // Serial.print    ("EPROM_NeedWrite 3 =");
            // Serial.println  (EPROM_NeedWrite);
    }
    else
    { 
        EPROM_NeedWrite=0;
    }    
};    


void EPROM_Read ()
{
    if (EEPROM.read(0)==111)
    {
        for (int i=0;i<5;i++)
        {
            EEPROM.get(i+1,LedSettings[i].targetChnlBright_percent);
            Menu[i+1].UpdateRow2_Value(); 
        }

    }
}


// процедура включает/выключает демо режим
void LedChnlBright_TestOnOFF(int8_t increment)
{
    if (timerOnOff==1) // Если таймеры включены
    {
        switch (increment)
        {
            case -1:
                demoMode = demoMode+increment==0?0:1;
                break;
            case 1:
                demoMode = demoMode+increment==1?1:0;
                break;
        }

        if (demoMode==0)  // демо режим выключен
        {   
            {        
                size_t destination_size = sizeof (mnuLedBrtTest);
                strncpy(mnuLedBrtTest, TmpChr, destination_size);
                mnuLedBrtTest[destination_size - 1] = '\0';
            }
            LCD_Update();
        }
    
        Tlc.clear();
        Tlc.update();

        #ifdef DEBUG_DemoMode
            Serial.println();
            Serial.print("demoMode=");Serial.println(demoMode);
            delay(1500);   
        #endif 
    }   
};

// процедура устанавливает/записывает в Tlc5940 текущее (расчётное, абсолютное) значение яркости канала
void LedChnlBright_Set(const uint32_t &currentMillis)
{

    // заплата, в рабочем коде убрать
    // для теста делаем так чтобы яркость всега была полной. 
     timerOnOff=0;



     
    if (timerOnOff==1) // если таймеры включены мы считаем яркость с учётом текущего/тестового (demoMode==1) времени
    {
            static uint8_t tmpHour=0;
            static uint8_t tmpMinute=0;   
            if (demoMode==1) // если включен демо режим то начинаем считать и выводить на экран "виртуальное" время 
            {
                //memoryReport("DemoMode_Start");
                // delay(10);
                    #ifdef DEBUG_DemoMode
                        Serial.println (F("_____DemoMode_ON____"));
                        delay (1500);
                    #endif

                    TimerPrevMillis[3] = currentMillis;
                    tmpMinute +=1; //+2                             // тут можно задать шаг приращения тестовых минут
                    if (tmpMinute >=60)
                    {
                        tmpHour +=1; tmpMinute =0;
                    } 
                    if (tmpHour >23) {tmpHour=0;}

                    TimerPrevMillis[2]= currentMillis;              // блокируем отключение подсветки
                    
                    #pragma region             
                    // // Вариант 1 реализации генерации строки тестового меню. Результат не очень...
                    // if (Menu[activeScreenNumber].GetMenuType() == MyMENU::TestLedChnlBright)
                    // {
                    //     lcd.setCursor(0, 0);
                    //     lcd.print("Test ");
                    //     lcd.setCursor(5, 0);
                    //     lcd.print(tmpHour);
                    //     lcd.setCursor(7, 0);
                    //     lcd.print(":");
                    //     lcd.setCursor(8, 0);
                    //     lcd.print(tmpMinute);
                    //     lcd.setCursor(10, 0);
                    //     lcd.print("      ");

                    // }
                    #pragma endregion

                    // Вариант 2 реализации генерации строки тестового меню. Результат ОК.
                    char tmpI3[17]="Test ";                         // генерируем временное значение первой строки данного экрана с информацией о параметрах теста
                    {
                        char t[3];
                        sprintf(t,"%u",tmpHour);
                        strcat(tmpI3, t);
                    }

                    strcat(tmpI3, ":");
                    {
                        char t1[3];
                        dtostrf(tmpMinute, 2, 0, t1); // не знаю почему но 2 sprintf в одной функции работать отказались. Пришлось использовать dtostrf
                        strcat(tmpI3, t1);
                    }
                    strcat(tmpI3, "      \0");
                    // копируем (!) полученное значение (массив char tmpI3) в переменную mnuLedBrtTest.
                    // копируем поскольку при передаче указателя на tmpI3, по завершеню работы функции tmpI3 выйдет из зоны видимости 
                    size_t destination_size = sizeof (mnuLedBrtTest);
                    strncpy(mnuLedBrtTest, tmpI3, destination_size);
                    mnuLedBrtTest[destination_size - 1] = '\0';


                    //snprintf(mnuLedBrtTest, 17, "%s", tmpI3);
                    // Menu[8].UpdateRow(1, tmpI3);
                    LCD_Update();
            }
            else
            {
                    #ifdef DEBUG_DemoMode
                        Serial.println (F("_____DemoMode_OFF____"));
                    #endif

                tmpHour     = time.Hours;       // time.Hours - Время в 24 формате, time.hours - время в 12 часовом формате
                tmpMinute   = time.minutes;
                // Serial.print(tmpHour);Serial.print(":");Serial.println(tmpMinute);
            }

            // обновляем рассчётное значение яркости всех ЛЕД каналов с учётом установленных таймеров включения/выключения и параметров рассвета/заката
            for (int i=0;i<5;i++)
            {
                LedSettings[i].update_PWM_Level (tmpHour, tmpMinute);
            }

    }
    else // вариант когда мы выключили таймеры: яркость каналов = установленному значению targetChnlBright_percent в абсолютном значении 
    {
            for (int i=0;i<5;i++)
            {
                LedSettings[i].update_PWM_Level_1();
            }

    }
            //Передаём рассчитанную "яркость" в tlc5940 для управления драйверами светодиодов (в tlc5940 16 каналов):
            // в текущей схеме используются "ноги" №№ 1,2,3,4,11
            Tlc.set(1,  LedSettings[0].PWM_channel_level); //(uint16_t)
            Tlc.set(2,  LedSettings[1].PWM_channel_level);
            Tlc.set(3,  LedSettings[2].PWM_channel_level);
            Tlc.set(4,  LedSettings[3].PWM_channel_level);   
            Tlc.set(11, LedSettings[4].PWM_channel_level);
            Tlc.update();



}

#ifdef SetSystemTime  //процедура нужна для установки системного времени в модуль RTC  в блоке void setup(void)
        void SetRTC_SysDateTime()
        {
            char* strDate =__DATE__;
            char* strTime =__TIME__;
            char s_month[5];
            const char month_names[] = "JanFebMarAprMayJunJulAugSepOctNovDec";            
            int Hour, Min, Sec;
            int month, day, year;


            sscanf(strTime, "%d:%d:%d", &Hour, &Min, &Sec); 
            sscanf(strDate, "%s %d %d", s_month, &day, &year);
            month = (strstr(month_names, s_month)-month_names)/3+1;
            
           // Hour =Hour +5; // Заплата для теста удалить

        #ifdef Debug_SetSystemTime
            Serial.println(F("comp DateTime parsing result: "));
            Serial.print(F("Day="));Serial.println(day);
            
            Serial.print(F("s_month ="));
            Serial.println(s_month);
            Serial.println(strstr(month_names, s_month));
            
            Serial.print(F("Month="));Serial.println(month);
            Serial.print(F("Year="));Serial.println(year);

            Serial.print(F("Hour="));Serial.println(Hour);
            Serial.print(F("Min="));Serial.println(Min);



            Serial.print (F("DateTime in RTC before adjastment: "));   
            Serial.println(time.gettime("d-m-Y, H:i:s, D")); 
        #endif          

        #ifdef Debug_SetSystemTime
            Serial.print (F("time.day="));   Serial.println (time.day);
            Serial.print (F("time.month="));   Serial.println (time.month);
            Serial.print (F("time.year="));   Serial.println (time.year);                        
        #endif   

        uint32_t rtcDate    = dayFromXmass(time.day,time.month, (time.year+2000));
        uint32_t compDate   = dayFromXmass(day, month, year);

        #ifdef Debug_SetSystemTime
            Serial.print (F("rtcDate="));   Serial.println (rtcDate);
            Serial.print (F("compDate="));   Serial.println (compDate);
        #endif   



       
        if (
            (compDate > rtcDate) ||
            ((compDate == rtcDate) && (Hour*60U*60U + Min*60U + Sec) > (time.hours*60U*60U+time.minutes*60U+time.seconds))
           )
        {
            time.settime(Sec,Min,Hour,day,month,year-2000,-1);   // установить время: 00 сек, 01 мин, 02 часа, 03 день, 04 месяц, 05 год, 06 день недели - суббота
                                                                // Если указать отрицательное значение, то соответствующий параметр останется без изменений.
            #ifdef Debug_SetSystemTime     
                Serial.println (F("Comp_Date > RTC_Date"));
                Serial.print (F("compDate="));Serial.println (compDate);
                Serial.print (F("rtcDate="));Serial.println (rtcDate);
            #endif        
        }                                                   // Например: time.settime(10,20,-1,-1,-1,-1,-1); приведёт к изменению секунд и минут, а часы и дата останутся без изменений          

        #ifdef Debug_SetSystemTime   
        else
        {
            Serial.println (F("RTC_Date/Time > CompDateTime"));

        }
        #endif


        #ifdef Debug_SetSystemTime          
            Serial.print (F("DateTime in RTC after  adjastment: "));   
            Serial.println(time.gettime("d-m-Y, H:i:s, D"));
        #endif              
        }
#endif

void LCD_Update()
{

    // обновляем  актуальный (activeScreenNumber) lcd экран
    
    //    lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(Menu[activeScreenNumber].GetRow(1));
        lcd.setCursor(0, 1);
        lcd.print(Menu[activeScreenNumber].GetRow(2));

        //  Serial.println();
        //     Serial.print ("Menu[activeScreenNumber].GetRow(1)=") ;      
        //     Serial.println (Menu[activeScreenNumber].GetRow(1)) ; 
        //     Serial.println(); Serial.println();
        //     Serial.print ("Menu[activeScreenNumber].GetRow(2)=") ;      
        //     Serial.println (Menu[activeScreenNumber].GetRow(2)) ;      
}

// процедура обработки нажатия кнопок
void ButtonClick(int k) 
{
    if (k>-1) // если кнопка нажата 
    {
        switch (k) 
            { 
            case -1: {break;}                                                               // Кнопка Не нажата   
                
            case 0: {                                                                       // Нажата кнопка -> / Вперед 
                    #ifdef DEBUG_Func_buttons_control
                        Serial.println("Key 0 pressed");
                    #endif
                    
                    if (CurrOperationMode==MyMENU::NotEdit)                                 // если мы НЕ находимся в режиме редактирования 
                    {
                            // Serial.print("activeScreenNumber_Before=");
                            // Serial.println(activeScreenNumber);
                            activeScreenNumber= activeScreenNumber+1>=MyMENU::MenuAmount?0:activeScreenNumber+1;
                            // Serial.print("activeScreenNumber_After=");
                            // Serial.println(activeScreenNumber);
                    }
                    else if(CurrOperationMode==MyMENU::RowEdit)                             // если мы находимся в режиме редактирования ()
                        {
                            Menu[activeScreenNumber].ptr_on_click(1);
                            Menu[activeScreenNumber].UpdateRow2_Value();                                          
                        }    
                    break;
                    } 

            case 2: {                                                                           // Нажата кнопка <- /Назад
                    #ifdef DEBUG_Func_buttons_control
                        Serial.println("Key 2 pressed");
                    #endif

                        if (CurrOperationMode==MyMENU::NotEdit)                                 // если мы не находимя в режиме редактирования 
                        {   
                                        // Serial.print("activeScreenNumber_Before=");
                                        // Serial.println(activeScreenNumber);
                            activeScreenNumber= activeScreenNumber-1<0?MyMENU::MenuAmount-1:activeScreenNumber-1;
                                        // Serial.print("activeScreenNumber_After=");
                                        // Serial.println(activeScreenNumber);
                        } // текущее меню -1
                        else if(CurrOperationMode==MyMENU::RowEdit)                             // если мы находимся в режиме редактирования ()
                        {
                            Menu[activeScreenNumber].ptr_on_click(-1);
                            Menu[activeScreenNumber].UpdateRow2_Value();      
                        }
                    // else {};    
                    break;
                    }

            case 1: {                                                                        // Нажата кнопка  ^ / Вверх / Select
                    #ifdef DEBUG_Func_buttons_control
                        Serial.println("Key 1 pressed");
                    #endif

                    if (CurrOperationMode!=MyMENU::RowEdit)                              // если мы не находимя в режиме редактирования строки, то переключаемся в режим редактирования
                        {
                            CurrOperationMode = static_cast<MyMENU::t_MenuOperationMode>(static_cast<int>(CurrOperationMode)+1);
                                    // Serial.print ("CurrOperationMode'+1'=");
                                    // Serial.print (CurrOperationMode);
                        }
                    break;}
                    
            case 3: {// Нажата кнопка "вниз"/Сancel 
                    #ifdef DEBUG_Func_buttons_control
                        Serial.println("Key 3 pressed");
                    #endif
                    
                    if (CurrOperationMode==MyMENU::NotEdit)                                 // если мы не находимся в режиме редактирования 
                        {activeScreenNumber=0;}                                          //Возврат в главное меню (Дата и температура)
                    else                                                                    // иначе Понижаем уровень редактирования
                        {CurrOperationMode = static_cast<MyMENU::t_MenuOperationMode>(static_cast<int>(CurrOperationMode)-1);
                                    // Serial.print ("CurrOperationMode'-1'=");
                                    // Serial.print (CurrOperationMode);
                        }                                
                    break;}
            };
            LCD_Update();
            lcd.backlight(); TimerPrevMillis[2]=millis();
    }
}




// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------







void setup(void)
{       

    #ifdef DEBUG_Setup
        Serial.begin(SERIAL_BAUD);
        Serial.println(F("Debuging start......4..."));
        Serial.println();
        Serial.println();
    #endif

    // memoryReport("setup_Start");

    lcd.init();                           // Инициализация объекта I2C LCD.
    lcd.backlight();

    //Инициализируем  Tlc5940
    Tlc.init();
    Tlc.clear();


    // настройка модуля RTC
    time.begin();                   // инициируем RTC модуль
    time.period(1);               // Указываем обращаться к модулю RTC не чаще чем один раз в минуту

    // устанавливаем время в RTC = системному времени и дате в момент компиляции 
    // (при условиия что системмное время (=времени компиляции) больше времени в модуле RTC)
    #ifdef SetSystemTime                
        SetRTC_SysDateTime();        
    #endif                              
    #ifdef Debug_SetSystemTime
        Serial.println (F("Init RTS Module - OK"));    
        Serial.print (F("Time in RTS Module is:"));
        Serial.println(time.gettime("d-m-Y, H:i:s, D")); 
    #endif

    // настраиваем порты
        pinMode(portDC18B20PWM_1, OUTPUT);
        pinMode(portDC18B20PWM_2, OUTPUT);
        digitalWrite(portDC18B20PWM_1, LOW);
        digitalWrite(portDC18B20PWM_2, LOW);


    // инициализируем датчики температуры и устанавливаем порог температуры срабатывания
        DS18_settings.TargetTemp_D1 =DC18_MaxGoodTemp;
        DS18_settings.TargetTemp_D2 =DC18_MaxGoodTemp;
        if (DS1822_init(DS18_OneWare, DS18_settings))  
            {
               #ifdef DEBUG_DS18B20
                  Serial.println ("Init DC18B20 - OK");
                #endif 
                //Errors[1].codeErr=0;               
            }
            else 
            {
                #ifdef DEBUG_DS18B20
                Serial.println ("Init DC18B20 - Error");
                //  Serial.println (Errors[1].TextErr);
                #endif  
                //Errors[1].codeErr=1;
            } 



    LCD_Update();

    // for (int i =0; i< MyMENU::MenuAmount;i++)
    // {Menu[i].DebugPrint();}

    //    EPROM_Write(); 
    EPROM_Read();




    // Задаём базовые параметры включения/выключения всех ЛЕД каналов 
    // Параметры - Номер таймера 0/1; час включения; минуты включения; "длина" включения,  аналогично выключение 
    // for (int i =0; i<5; i++)
    //     {
    //         LedSettings[i].SetTimer(0,7,30,30,12,0,10); 
    //         LedSettings[i].SetTimer(1,16,0,1,22,0,15);
    //     }

    LedSettings[0].SetTimer(0,7,30,30,12,0,10); 
    LedSettings[0].SetTimer(1,16,0,1,21,20,30);

    LedSettings[1].SetTimer(0,7,00,30,12,30,10); 
    LedSettings[1].SetTimer(1,16,0,1,21,30,5);

    LedSettings[2].SetTimer(0,8,00,50,11,0,10); 
    LedSettings[2].SetTimer(1,16,0,1,22,0,5);

    LedSettings[3].SetTimer(0,8,00,10,13,0,35); 
    LedSettings[3].SetTimer(1,16,0,1,21,10,10);

    LedSettings[4].SetTimer(0,8,00,20,14,0,45); 
    LedSettings[4].SetTimer(1,16,0,1,21,30,5);





    // memoryReport("setup_End");
    delay (1000);  // удалить
}




void loop(void)
{
 
    uint32_t currentMillis = millis();
    


    // обновление/расчёт параметров яркости для ЛЕД каналов
    if (currentMillis - TimerPrevMillis[3] >= TimeInterval_LedCNL_Test)
    {
        //  memoryReport("CurrLedBrght_BeforeCall");
            LedChnlBright_Set(currentMillis);
        //  memoryReport("CurrLedBrght_AfterCall");
    }


   





    // обновляем дату/время в Menu
    if (currentMillis - TimerPrevMillis[0] >= TimeInterval_RTC)
    {
            TimerPrevMillis[0] = currentMillis;
            Menu[0].UpdateRow(1, time.gettime("d.m.Y H:i"));
            LCD_Update();
        // #ifdef Debug_SetSystemTime
        //         Serial.println(">=TimeInterval_RTC");
        //         Serial.print("CurrentDateTime=");
        //         Serial.println(time.gettime("d.m.Y, H:i:s, D"));
        // #endif
    }

    // опрос датчиков температуры
    if (currentMillis - TimerPrevMillis[1] >= TimeInterval_DS18D20)
    {
        if (DS18_sensorRequest(DS18_OneWare, DS18_settings))
        {
            TimerPrevMillis[1] = currentMillis;
            Menu[0].UpdateRow(2, "t1=", &DS18_settings.CurrentTemp_D1,"  t2=",  &DS18_settings.CurrentTemp_D2);      
            LCD_Update();
        #ifdef DEBUG_DS18B20
                    Serial.print(F("TempRead.T1="));
                    Serial.println(DS18_settings.CurrentTemp_D1);
                    Serial.print(F("TempRead.T2="));
                    Serial.println(DS18_settings.CurrentTemp_D2);
        #endif
        }
    }
    if (DS18_settings.D1_tempCounter == 0xFF) 
        {   //0xFF =255 или  11111111 в бинарном. При каждои привышении граница температуры пишем 1 в новый бит если получаем 0xFF то значит темперетура превышена на протяжении заданного интервала, введено для избежания "дребезга" 
            DS18_settings.PWM_D1_Level = (DS18_settings.CurrentTemp_D1 - DS18_settings.TargetTemp_D1)*DS18_settings.PWM_StepUP;
            if (DS18_settings.PWM_D1_Level>255){DS18_settings.PWM_D1_Level=255;} //t1 -- счетчик увеличения уровня ШИМ 
            #ifdef DEBUG_DS18B20
                Serial.print (F("D1_Current temp: "));
                Serial.print (DS18_settings.CurrentTemp_D1,5);
                Serial.print (F(",   D1_Target temp: "));
                Serial.print (float(DS18_settings.TargetTemp_D1),5);
                Serial.print (F(",  PWM_1 level: "));
                Serial.println (float(DS18_settings.PWM_D1_Level),2);
            #endif
            analogWrite(portDC18B20PWM_1, DS18_settings.PWM_D1_Level); // записываем значение ШИМ в порт   
        }  
        else {DS18_settings.PWM_D1_Level=0;}

    if (DS18_settings.D2_tempCounter == 0xFF) 
        {
            DS18_settings.PWM_D2_Level = (DS18_settings.CurrentTemp_D2 - DS18_settings.TargetTemp_D2)*DS18_settings.PWM_StepUP;
            if (DS18_settings.PWM_D2_Level>255){DS18_settings.PWM_D2_Level=255;} 
            #ifdef DEBUG_DS18B20
                Serial.print (F("D2_Current temp: "));
                Serial.print (DS18_settings.CurrentTemp_D2,5);
                Serial.print (F(",   D2_Target temp: "));
                Serial.print (float(DS18_settings.TargetTemp_D2),5);
                Serial.print (F(",  PWM_2 level: "));
                Serial.println (float(DS18_settings.PWM_D2_Level),2);
            #endif
    
            analogWrite(portDC18B20PWM_2, DS18_settings.PWM_D2_Level); // записываем значение ШИМ в порт    
        }  
        else {DS18_settings.PWM_D2_Level=0;}





    

    // --- LCD Подсветка
    if (currentMillis - TimerPrevMillis[2] >= TimeInterval_LCDBacklight)
    {
        TimerPrevMillis[2] = currentMillis;
        lcd.noBacklight();
    }




    // обработка нажатия кнопки
    // скорость нажатия решулируется переменной #define KeyNotReactInterval 
    int8_t k = MyButtons.KeyPressedCode(currentMillis);    
    if (k>-1) // если кнопка нажата 
        {   
            ButtonClick(k);
        }
        else if (k==-1)
        {
            //   Serial.println("MyButtons.KeyPressedCode=-2");
        }
        else 
        {
             Serial.print(F("MyButtons.KeyPressedCode="));Serial.println(k);
        }



 
 
 
}

