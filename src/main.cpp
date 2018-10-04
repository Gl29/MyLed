//------- Библиотеки -------------------------------------------------------------------------------------
#include <Arduino.h>
#include <OneWire.h>            //Библиотека 1-Wire. для цифровых датчиков температуры 
//#include <Wire.h>
//#include <RTClib.h>
// #include <TimeLib.h>
// #include <DS1307RTC.h>       //a basic DS1307 library that returns time as a time_t. 

#include <LiquidCrystal_I2C.h>  //Библиотека I2C для LCD
#include <iarduino_RTC.h>       // подключаем библиотеку для работы с RTC модулем https://lesson.iarduino.ru/page/urok-17-podklyuchenie-rtc-chasy-realnogo-vremeni-s-knopkami/
                                // 

// #include <EEPROM.h>          // Библиотека для работы с EEPROM 
// #include <leOS.h>            // Шедуллер задач -- если запущено несколько задач начинаются тормоза, пробуем без него

#include <LiquidMenu.h>         // Библиотека для работы с меню -- каждый экран "жрёт" много памяти

//----- Самописныйе блоки кода, дабы не писать всё в Main ---------------------------------------------------
#include "MyLCD.h"              // вспомогательные функции для вывода на экран 
#include "Buttons.h"            // работа с кнопками
#include "MyAddFunctions.h"     // сборник дополнительных функций вынесен в отдельный файл "дабы не засорять эфир" 
#include "MyDC18B20.h"          // код для работы с датчикками темперетуры
#include "LedChannel.h"         // код для работы с LED каналами

#define DEBUG_Setup             // Включаем общий режим отладки
//#define DEBUG_DS18B20           // Включаем режим отладки Датчика температуры
//#define DEBUG_Loop            // Включаем режим отладки для функции Void LOOP
#define SetSystemTime           // Включает режим установки системного времени
//#define Debug_SetSystemTime     // Включает режим отладки в момент установки системного времени

//-------- Стартовые параметры -----------------------------------------------------------------------
#define SERIAL_BAUD 9600
                                            //#define INTERVAL2  5*60*1000UL
#define TimeInterval_DS18D20        10*1000 //15000 //10000 // Время опроса датчиков температуры м.сек
#define TimeInterval_RTC            10*1000 // периодичность обновления информации о текущем времени м.сек
#define TimeInterval_LCDUpdate      100     // Периодичность обновления экрана м.сек
#define TimeInterval_LCDBacklight   10*1000 // Время через которое выключается подсветка экрана м.сек
#define DC18_MaxGoodTemp            30      // Максимальная температура после превышения которой срабатывает событие (градусы цельсия)


//-------- PINs к которым прицеплена переферия --------------------------------------------------------
#define portOneWire 2                   // указаываем порт OneWire   протокол 1-Wire
#define portDC18B20PWM_1 5              // указаываем порт для PWM DC18B20_1  
#define portDC18B20PWM_2 6              // указаываем порт для PWM DC18B20_2
#define pordButtonsRead A0              // указаываем порт на котором будем слушать нажатие кнопок


//-------- Глобальные переменные -----------------------------------------------------------------------
//iarduino_RTC time(RTC_DS1307);        // объявляем переменную time для работы с библиотекой, указывая название модуля RTC_DS1307
//leOS myTask;                          //create a new istance of the class leOS
//tmElements_t tm;                      // переменная которая хранит время в формте TimeLib.h 
//MyLCD  LCD_1602(0x27,16,2);           // инициируем экран //int(0x27)
OneWire DS18_OneWare(portOneWire);      // инициируем OneWire протокол 1-Wire
struct_DS18_setting DS18_settings;      // структра для хранения набора параметров для датчиков температуры 
stuct_LedBrightness LedSettings [5] = {{"White6300_1",1,1000},{"White6300_2",2,1000},{"White4500",3,1000},{"Red",4,},{"Blue",5,1000},};
Err Errors[] ={"Error read RTS",false,"Error init DC18B20", false}; // структра для хранения кодов ошибок
CurrDateTimeSTR CurrentDateTime;        // структра для хранения текущих времени и даты в текстовом виде и в CHAR*

LiquidCrystal_I2C lcd(0x27, 16, 2);     // объявляем  переменную lcd для работы с LCD дисплеем, указывая параметры дисплея (адрес I2C = 0x27, количество столбцов = 16, количество строк = 2)
Button MyButtons = Button(A0);          // Обьект для отработки нажатия кнопок
uint32_t TimerPrevMillis[4];            // Массив для хранения времени прошедшего с момента последнего срабатывания таймеров
                                        // Таймеры:
                                        // 0 - Считывание времени из RTC
                                        // 1 - Опрос датчиков температуры
                                        // 2 - Включение подсветки
                                        // 3 - Обновление экрана
iarduino_RTC time(RTC_DS1307);          // объявляем переменную time для работы с библиотекой, указывая название модуля RTC_DS1307                              
uint8_t VAR_screen_NUMBER   = 1;        // тип экрана: 1-время/дата/темперетура 
                                        // 2-  Яркость LED линии 1
                                        // 3 - Яркость LED линии 2
                                        // 4-6 Яркость LED линий 3-5


uint8_t VAR_mode_SET        = 1;        // режим установки времени: 0-нет 1-нет (секунды сбрасываем автоматом) 2-мин 3-час 4-день 5-мес 6-год 7-день_недели //1-сек

// void DateTimeUpdate() // обновляем текущие дату и время. (используется в leOS myTask)
//     {

        
//         if (RTC.read(tm)) { 
//                 GetSTR_DateTime(tm, CurrentDateTime);
//                 Errors[0].codeErr=0;}
//         else {
//                 Errors[0].codeErr=1;} 
//     };




#ifdef SetSystemTime  // процедура нужна для установики системного времени в модуль RTC  в блоке void setup(void)
        void SetRTC_SysDateTime()
        {
            char* strDate =__DATE__;
            char* strTime =__TIME__;
 
            char s_month[5];
            static const char month_names[] = "JanFebMarAprMayJunJulAugSepOctNovDec";            
            int Hour, Min, Sec;
            int month, day, year;

            sscanf(strTime, "%d:%d:%d", &Hour, &Min, &Sec); 
            sscanf(strDate, "%s %d %d", s_month, &day, &year);
            month = (strstr(month_names, s_month)-month_names)/3;
        #ifdef Debug_SetSystemTime
            Serial.println("System DateTime parsing result: ");
            Serial.print("Day=");Serial.println(day);
            Serial.print("Month=");Serial.println(month);
            Serial.print("Year=");Serial.println(year);

            Serial.print ("DateTime in RTC before adjastment: ");   
            Serial.println(time.gettime("d-m-Y, H:i:s, D")); 
        #endif            
            time.settime(Sec,Min,Hour,day,month,year-2000,-1);   // установить время: 00 сек, 01 мин, 02 часа, 03 день, 04 месяц, 05 год, 06 день недели - суббота
                                                            // Если указать отрицательное значение, то соответствующий параметр останется без изменений.
                                                            // Например: time.settime(10,20,-1,-1,-1,-1,-1); приведёт к изменению секунд и минут, а часы и дата останутся без изменений          
        #ifdef Debug_SetSystemTime          
            Serial.print ("DateTime in RTC after  adjastment: ");   
            Serial.println(time.gettime("d-m-Y, H:i:s, D"));
        #endif              
        }
#endif


// Функция управления кнопками:
void Func_buttons_control(int8_t PressedKey,uint64_t currentMillis)
{   
    Serial.println();
    Serial.println ("Func_buttons_control... Starting");
    Serial.print("PressedKey="); Serial.println(PressedKey);
    Serial.print("BaseVAR_screen_NUMBER="); Serial.println(VAR_screen_NUMBER);
    Serial.print("BaseVAR_mode_SET=");  Serial.println(VAR_mode_SET);   
    
    lcd.backlight();
    TimerPrevMillis[2]=currentMillis;

        //  Если нажата кнопка Cancel - выходим из режима редактирования
        if(PressedKey==3) {VAR_mode_SET=1;       // отключаем режим редактирования (0 или 1)
                           VAR_screen_NUMBER=1;  // возвращаемся на главный экран
                           time.blinktime(0);    // выключаем мигание
                            Serial.print("ModVAR_screen_NUMBER="); Serial.println(VAR_screen_NUMBER);
                            Serial.print("ModVAR_mode_SET=");  Serial.println(VAR_mode_SET);   
                            Serial.println("worked branch - 0 -Cancel");}  


        if(PressedKey ==1)
        {                 //  Если нажата кнопка SET/OK - переходим в режим редактирования
            Serial.println("worked branch - 1 - Вкл. Режим Редактирования"); 
            switch (VAR_screen_NUMBER)
            {
                case 1: VAR_mode_SET++; VAR_mode_SET= VAR_mode_SET>=7?2:VAR_mode_SET;break;       //Если часы находятся в режиме вывода даты+времени+Температуры
                case 2: VAR_mode_SET=8;                                             //Редактирование экрана 2
                case 3: VAR_mode_SET=9;                                             //Редактирование экрана 3
                case 4: VAR_mode_SET=10;                                            //Редактирование экрана 4    
                case 5: VAR_mode_SET=11;                                            //Редактирование экрана 5    

            }
            time.blinktime(VAR_mode_SET,5);         // мигаем устанавливаемым параметром (если VAR_mode_SET больше 0)
                     //переменная = условие ? выражение1 : выражение2
                     //Этот оператор проверяет условие и, если оно выполняется, то вычисляется значение 
                     //выражения1 и результат вычисления присваивается переменной, 
                     //а если условие не выполняется, то вычисляется значение выражения2 
                     //и результат вычисления присваивается переменной.       
                        Serial.print("ModVAR_screen_NUMBER="); Serial.println(VAR_screen_NUMBER);
                        Serial.print("ModVAR_mode_SET=");  Serial.println(VAR_mode_SET); 
                        Serial.println("worked branch - 1 - Вкл. Режим Редактирования");                                                      
        } //  Если нажата кнопка SET/OK - переходим в режим редактирования
        

        if(PressedKey==2 && VAR_mode_SET <2) {    // переключаемся на предыдущий экран
                           VAR_screen_NUMBER--;
                           VAR_screen_NUMBER= VAR_screen_NUMBER<1?5:VAR_screen_NUMBER;
                           Serial.print("ModVAR_screen_NUMBER="); Serial.println(VAR_screen_NUMBER);
                           Serial.print("ModVAR_mode_SET=");  Serial.println(VAR_mode_SET);                            
                           Serial.println("worked branch - 10 - Prev.Screen. NotEdit");}          
        if(PressedKey==0 && VAR_mode_SET <2) {    // переключаемся на Next экран
                           VAR_screen_NUMBER++;
                           VAR_screen_NUMBER= VAR_screen_NUMBER>5?1:VAR_screen_NUMBER;
                           Serial.print("ModVAR_screen_NUMBER="); Serial.println(VAR_screen_NUMBER);
                           Serial.print("ModVAR_mode_SET=");  Serial.println(VAR_mode_SET);                            
                           Serial.println("worked branch - 11 - Next.Screen. NotEdit");}          
         

        if(VAR_mode_SET>1){
            
            //  Если нажата кнопка UP/Вперёд
            if(PressedKey==0){
            switch (VAR_mode_SET){                                     // инкремент (увеличение) устанавливаемого значения
            ///* сек */ case 1: time.settime(0,                                   -1, -1, -1, -1, -1, -1); break;
            /* мин */       case 2: time.settime(0, (time.minutes==59?0:time.minutes+1), -1, -1, -1, -1, -1); break;
            /* час */       case 3: time.settime(0, -1, (time.Hours==23?0:time.Hours+1),     -1, -1, -1, -1); break;
            /* дни */       case 4: time.settime(0, -1, -1, (time.day==31?1:time.day+1),         -1, -1, -1); break;
            /* мес */       case 5: time.settime(0, -1, -1, -1, (time.month==12?1:time.month+1),     -1, -1); break;
            /* год */       case 6: time.settime(0, -1, -1, -1, -1, (time.year==99?0:time.year+1),       -1); break;
             //  /* д.н.*/ case 7: time.settime(0, -1, -1, -1, -1, -1, (time.weekday==6?0:time.weekday+1) ); break; 
 
            /* Screen2*/    case 8:  LedSettings[0].channalBrightness = 
                                        LedSettings[0].channalBrightness >=2000?2000:LedSettings[0].channalBrightness++;; break;
            /* Screen3*/    case 9:  LedSettings[1].channalBrightness = 
                                        LedSettings[1].channalBrightness >=2000?2000:LedSettings[1].channalBrightness++;; break;
            /* Screen4*/    case 10: LedSettings[2].channalBrightness = 
                                        LedSettings[2].channalBrightness >=2000?2000:LedSettings[2].channalBrightness++;; break;
            /* Screen5*/    case 11: LedSettings[3].channalBrightness = 
                                        LedSettings[3].channalBrightness >=2000?2000:LedSettings[3].channalBrightness++;; break;
                }
            Serial.println("worked branch - 3 - UP/Вперёд");    
            }


            //  Если нажата кнопка DOWN/Назад
            if(PressedKey==2){
                switch (VAR_mode_SET){                                     // декремент (уменьшение) устанавливаемого значения
            ///* сек */ case 1: time.settime(0,                                   -1, -1, -1, -1, -1, -1); break;
            /* мин */       case 2: time.settime(0, (time.minutes==0?59:time.minutes-1), -1, -1, -1, -1, -1); break;
            /* час */       case 3: time.settime(0, -1, (time.Hours==0?23:time.Hours-1),     -1, -1, -1, -1); break;
            /* дни */       case 4: time.settime(0, -1, -1, (time.day==1?31:time.day-1),         -1, -1, -1); break;
            /* мес */       case 5: time.settime(0, -1, -1, -1, (time.month==1?12:time.month-1),     -1, -1); break;
            /* год */       case 6: time.settime(0, -1, -1, -1, -1, (time.year==0?99:time.year-1),       -1); break;
           // /* д.н.*/     case 7: time.settime(0, -1, -1, -1, -1, -1, (time.weekday==0?6:time.weekday-1) ); break;
            /* Screen2*/    case 8:  LedSettings[0].channalBrightness = 
                                        LedSettings[0].channalBrightness <=0?0:LedSettings[0].channalBrightness--;; break;
            /* Screen3*/    case 9:  LedSettings[1].channalBrightness = 
                                        LedSettings[1].channalBrightness <=0?0:LedSettings[1].channalBrightness--;; break;
            /* Screen4*/    case 10: LedSettings[2].channalBrightness = 
                                        LedSettings[2].channalBrightness <=0?0:LedSettings[2].channalBrightness--;; break;
            /* Screen5*/    case 11: LedSettings[3].channalBrightness = 
                                        LedSettings[3].channalBrightness <=0?0:LedSettings[3].channalBrightness--;; break;

            Serial.println("worked branch - 4 - DOWN/Назад");
            }
            //  Если нажата кнопка SET/OK
            // if(PressedKey==1){
            //     VAR_mode_SET++;
            //     if(VAR_mode_SET>6){VAR_mode_SET=2;}     // переходим к следующему устанавливаемому параметру
            //     time.blinktime(VAR_mode_SET,5);         // мигаем устанавливаемым параметром (если VAR_mode_SET больше 0)
            //     Serial.println("worked branch - 5 - SET/OK");          
            // }
 
        }
    }
    Serial.println ("Func_buttons_control... END");
}







void setup(void)
{       
    #ifdef DEBUG_Setup
        Serial.begin(SERIAL_BAUD);
        Serial.println("Debuging start......3.....................");
        Serial.println();
        Serial.println();
    #endif

  lcd.init();                           // Инициализация объекта I2C LCD.
  lcd.backlight();
  time.begin();                         // инициируем RTC модуль
  
    #ifdef SetSystemTime                // устанавливаем время в RTC = системному времени и дате в момент компиляции 
        SetRTC_SysDateTime();        
    #endif                              
    #ifdef Debug_SetSystemTime
        Serial.println ("Init RTS Module - OK");    
        Serial.print ("Время в модуле RTS:");
        Serial.println(time.gettime("d-m-Y, H:i:s, D")); 
    #endif

// настраиваем порты
        pinMode(portDC18B20PWM_1, OUTPUT);
        pinMode(portDC18B20PWM_2, OUTPUT);
        digitalWrite(portDC18B20PWM_1, LOW);
        digitalWrite(portDC18B20PWM_2, LOW);

    // инициализируем датчики температуры и устанавливаем порог температуры 
        DS18_settings.TargetTemp_D1 =DC18_MaxGoodTemp;
        DS18_settings.TargetTemp_D2 =DC18_MaxGoodTemp;
        if (DS1822_init(DS18_OneWare, DS18_settings))  
            {
               #ifdef DEBUG_DS18B20
                  Serial.println ("Init DC18B20 - OK");
                #endif 
                Errors[1].codeErr=0;               
            }
            else 
            {
                #ifdef DEBUG_DS18B20
                  Serial.println (Errors[1].TextErr);
                #endif  
                Errors[1].codeErr=1;
            } 
}


void loop(void)
{
    unsigned long currentMillis = millis();

    //TimerPrevMillis
if (currentMillis - TimerPrevMillis[0]>=TimeInterval_RTC)
                    {TimerPrevMillis[0]=currentMillis;
                    lcd.setCursor(0, 0); 
                     lcd.print(time.gettime("d.m.Y H:i"));
                    }    

if (currentMillis - TimerPrevMillis[3]>=TimeInterval_LCDUpdate)
    {TimerPrevMillis[3]=currentMillis;    
    switch (VAR_screen_NUMBER)
        {
            case 1:{ // Основной экран вывода даты/времени и температуры
    
                        lcd.setCursor(0, 0); 
                        lcd.print(time.gettime("d.m.Y H:i"));
                        #ifdef Debug_SetSystemTime
                            Serial.println(">=TimeInterval_RTC");
                            Serial.print("CurrentDateTime=");   
                            Serial.println(time.gettime("d.m.Y, H:i:s, D"));
                        #endif
                        
                    
                    if (currentMillis - TimerPrevMillis[1]>=TimeInterval_DS18D20)
                        {
                        if (DS18_sensorRequest (DS18_OneWare, DS18_settings))
                        {TimerPrevMillis[1]=currentMillis;
                            lcd.setCursor(0, 1);
                            lcd.print("D1="); lcd.print(DS18_settings.CurrentTemp_D1); 
                            lcd.print(" "); lcd.print("D2="); lcd.print(DS18_settings.CurrentTemp_D2); 
                        #ifdef DEBUG_DS18B20
                                Serial.print("TempRead. T1=");
                                Serial.println(DS18_settings.CurrentTemp_D1);
                                Serial.print("TempRead. T2=");
                                Serial.println(DS18_settings.CurrentTemp_D2); 
                        #endif                  
                        }
                }
                break;}

            case 2:{
                    lcd.setCursor(0, 0); 
                    lcd.print("CNL: ");
                    lcd.print(LedSettings[0].channelName);
                    lcd.print("        ");
                    lcd.setCursor(0, 1);
                    lcd.print("Bright= ");
                    lcd.print(LedSettings[0].channalBrightness);
                    lcd.print("        ");                    
                break;}
            case 3:{

                    lcd.setCursor(0, 0); 
                    lcd.print("CNL: ");
                    lcd.print(LedSettings[1].channelName);
                    lcd.print("        ");
                    lcd.setCursor(0, 1);
                    lcd.print("Bright= ");
                    lcd.print(LedSettings[1].channalBrightness);
                    lcd.print("        ");                    
                break;}
            case 4:{
                    lcd.setCursor(0, 0); 
                    lcd.print("CNL: ");
                    lcd.print(LedSettings[2].channelName);
                    lcd.print("        ");                    
                    lcd.setCursor(0, 1);
                    lcd.print("Bright= ");
                    lcd.print(LedSettings[2].channalBrightness);
                    lcd.print("        ");                    
                break;}
            case 5:{
                    lcd.setCursor(0, 0); 
                    lcd.print("CNL: ");
                    lcd.print(LedSettings[3].channelName);
                    lcd.print("        ");                    
                    lcd.setCursor(0, 1);
                    lcd.print("Bright= ");
                    lcd.print(LedSettings[3].channalBrightness);
                    lcd.print("        ");                    
                break;}
        }
    }
    // if (currentMillis - TimerPrevMillis[0]>=TimeInterval_RTC)
    //         {TimerPrevMillis[0]=currentMillis;
    //             if(VAR_screen_NUMBER < 3) //==1 // если установлен режим вывода на экран времени/температуры
    //             {                    
    //                 lcd.setCursor(0, 0); 
    //                 lcd.print(time.gettime("d.m.Y H:i"));
    //             }                        // получаем обновлённое время из RTC на экран
    //             #ifdef Debug_SetSystemTime
    //                 Serial.println(">=TimeInterval_RTC");
    //                 Serial.print("CurrentDateTime=");   
    //                 Serial.println(time.gettime("d.m.Y, H:i:s, D"));
    //             #endif
    //         }


          
        
    // if (currentMillis - TimerPrevMillis[1]>=TimeInterval_DS18D20)
    //         {
    //             if (DS18_sensorRequest (DS18_OneWare, DS18_settings))
    //                 {TimerPrevMillis[1]=currentMillis;
    //                 if(VAR_screen_NUMBER==1) // если установлен режим вывода на экран времени/температуры
    //                 {                    
    //                     lcd.setCursor(0, 1);
    //                     lcd.print("D1="); 
    //                     lcd.print(DS18_settings.CurrentTemp_D1); 
    //                     lcd.print(" ");
    //                     lcd.print("D2="); 
    //                     lcd.print(DS18_settings.CurrentTemp_D2); 
                        
    //                 }
    //                 #ifdef DEBUG_DS18B20
    //                         Serial.print("TempRead. T1=");
    //                         Serial.println(DS18_settings.CurrentTemp_D1);
    //                         Serial.print("TempRead. T2=");
    //                         Serial.println(DS18_settings.CurrentTemp_D2); 
    //                 #endif                  
    //                 }
    //         }

    if (currentMillis - TimerPrevMillis[2]>=TimeInterval_DS18D20)
            {TimerPrevMillis[2]=currentMillis;
            //lcd.noBacklight();
            }        


     
    int8_t k = MyButtons.KeyPressedCode();
      
    switch (k) 
     { Serial.println("switch (k)");
        case 0: {//menu.change_screen(1); 
                    Serial.println("Key 0 pressed");break;}// lcd.backlight(); TimerPrevMillis[2]=currentMillis; break;}
        case 1: {//menu.change_screen(2); 
                    Serial.println("Key 1 pressed");break;}//; lcd.backlight(); TimerPrevMillis[2]=currentMillis; break;}
        case 2: {//menu.change_screen(3); 
                    Serial.println("Key 2 pressed");break;}//; lcd.backlight(); TimerPrevMillis[2]=currentMillis; break;}
        case 3: {//menu.change_screen(4); 
                    Serial.println("Key 3 pressed");break;}//; lcd.backlight(); TimerPrevMillis[2]=currentMillis; break;}

    };
    if(k !=-1){Func_buttons_control(k,currentMillis);}  

    // switch (MyButtons.KeyPressedCode()) 
    //  {
    //     case 0: {//menu.change_screen(1); 
    //                 Serial.println("Key 0 pressed"); lcd.backlight(); TimerPrevMillis[2]=currentMillis; break;}
    //     case 1: {//menu.change_screen(2); 
    //                 Serial.println("Key 1 pressed"); lcd.backlight(); TimerPrevMillis[2]=currentMillis; break;}
    //     case 2: {//menu.change_screen(3); 
    //                 Serial.println("Key 2 pressed"); lcd.backlight(); TimerPrevMillis[2]=currentMillis; break;}
    //     case 3: {//menu.change_screen(4); 
    //                 Serial.println("Key 3 pressed"); lcd.backlight(); TimerPrevMillis[2]=currentMillis; break;}

    // };





  if (DS18_settings.D1_tempCounter == 0xFF) {   //0xFF =255 или  11111111 в бинарном. При каждои привышении граница температуры пишем 1 в новый бит если получаем 0xFF то значит темперетура превышена на протяжении заданного интервала, введено для избежания "дребезга" 
        DS18_settings.PWM_D1_Level = (DS18_settings.CurrentTemp_D1 - DS18_settings.TargetTemp_D1)*DS18_settings.PWM_StepUP;
        if (DS18_settings.PWM_D1_Level>255){DS18_settings.PWM_D1_Level=255;} //t1 -- счетчик увеличения уровня ШИМ 
        #ifdef DEBUG_DS18B20
            Serial.print ("D1_Current temp: ");
            Serial.print (DS18_settings.CurrentTemp_D1,5);
            Serial.print (",   D1_Target temp: ");
            Serial.print (float(DS18_settings.TargetTemp_D1),5);
            Serial.print (",  PWM_1 level: ");
            Serial.println (float(DS18_settings.PWM_D1_Level),2);
        #endif
        }  
    else {DS18_settings.PWM_D1_Level=0;}

  if (DS18_settings.D2_tempCounter == 0xFF) {
        DS18_settings.PWM_D2_Level = (DS18_settings.CurrentTemp_D2 - DS18_settings.TargetTemp_D2)*DS18_settings.PWM_StepUP;
        if (DS18_settings.PWM_D2_Level>255){DS18_settings.PWM_D2_Level=255;} 
        #ifdef DEBUG_DS18B20
            Serial.print ("D2_Current temp: ");
            Serial.print (DS18_settings.CurrentTemp_D2,5);
            Serial.print (",   D2_Target temp: ");
            Serial.print (float(DS18_settings.TargetTemp_D2),5);
            Serial.print (",  PWM_2 level: ");
            Serial.println (float(DS18_settings.PWM_D2_Level),2);
        #endif
    }  
    else {DS18_settings.PWM_D2_Level=0;}



    analogWrite(portDC18B20PWM_1, DS18_settings.PWM_D1_Level); // записываем значение ШИМ в порт   
    analogWrite(portDC18B20PWM_2, DS18_settings.PWM_D1_Level); // записываем значение ШИМ в порт

}
