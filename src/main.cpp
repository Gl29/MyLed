#include <Arduino.h>
#include <OneWire.h>        //Библиотека 1-Wire. для цифровых датчиков температуры 
//#include <Wire.h>
//#include <RTClib.h>
#include <TimeLib.h>
#include <DS1307RTC.h>      //a basic DS1307 library that returns time as a time_t. 

//#include <iarduino_RTC.h>   // подключаем библиотеку для работы с RTC модулем https://lesson.iarduino.ru/page/urok-17-podklyuchenie-rtc-chasy-realnogo-vremeni-s-knopkami/


// #include <EEPROM.h>      // Библиотека для работы с EEPROM 
//#include <leOS.h>           // Шедуллер задач
#include <LiquidCrystal_I2C.h> //Библиотека I2C для LCD
#include <LiquidMenu.h>     // Библиотека для работы с меню

#include "MyLCD.h"          // вспомогательные функции для вывода на экран 
#include "Buttons.h"        // работа с кнопками
#include "MyAddFunctions.h" // сборник дополнительных функций вынесен в отдельный файл "дабы не засорять эфир" 
#include "MyDC18B20.h"      // код для работы с датчикками темперетуры
#include "LedChannel.h"     // код для работы с LED каналами


#define DEBUG_Setup         // Включаем общий режим отладки
//#define DEBUG_Loop       // Включаем режим отладки для функции Void LOOP
#define SetSystemTime       // Включает режим отладки в момент установки системного времени


// стартовые параметры
#define SERIAL_BAUD 9600
                                            //#define INTERVAL2  5*60*1000UL
#define TimeInterval_DS18D20        10*1000   //15000 //10000 // Время опроса датчиков температуры м.сек
#define TimeInterval_RTC            10*1000    // периодичность обновления информации о текущем времени м.сек
#define TimeInterval_LCDBacklight   10*1000    // Время через которое выключается подсветка экрана м.сек

#define DC18_MaxGoodTemp            30      // Максимальная температура после превышения которой срабатывает событие (градусы цельсия)


//  PINs к которым прицеплена переферия ()
#define portOneWire 2                   // указаываем порт OneWire   протокол 1-Wire
#define portDC18B20PWM_1 5              // указаываем порт для PWM DC18B20_1  
#define portDC18B20PWM_2 6              // указаываем порт для PWM DC18B20_2
#define pordButtonsRead A0              // указаываем порт на котором будем слушать нажатие кнопок

//iarduino_RTC time(RTC_DS1307);          // объявляем переменную time для работы с библиотекой, указывая название модуля RTC_DS1307


//leOS myTask;                        //create a new istance of the class leOS
tmElements_t tm;                    // переменная которая хранит время в формте TimeLib.h 
OneWire DS18_OneWare(portOneWire);  // инициируем OneWire протокол 1-Wire
struct_DS18_setting DS18_settings;  // создаём экземпляр набора параметров для датчиков температуры 
stuct_LedBrightness LedSettings [5] = {{"White6300_1",1,1000},{"White6300_2",2,1000},{"White4500",3,1000},{"Red",4,},{"Blue",5,1000},};
Err Errors[] ={"Error read RTS",false,"Error init DC18B20", false};
CurrDateTimeSTR CurrentDateTime;
//MyLCD  LCD_1602(0x27,16,2);         // инициируем экран //int(0x27)

// // Объект I2C LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);
Button MyButtons = Button(A0);          // Класс отработки нажатия кнопок
uint32_t TimerPrevMillis[4];            //Массив для хранения времени прошедшего с момента последнего срабатывания таймеров
                                        // Таймеры:
                                        // 0 - Считывание времени из RTC
                                        // 1 - Опрос датчиков температуры
                                        // 2 - Включение подсветки
                                        // 3 - ?? пока резерв
                               





void DateTimeUpdate() // обновляем текущие дату и время. (используется в leOS myTask)
    {

        
        if (RTC.read(tm)) { 
                GetSTR_DateTime(tm, CurrentDateTime);
                Errors[0].codeErr=0;}
        else {
                Errors[0].codeErr=1;} 
    };




#ifdef SetSystemTime  // процедуры нужны для установики системного времени в модуль RTC  в блоке void setup(void)
        const char *monthName[12] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
        };

        bool getTime(const char *str)
        {
            int Hour, Min, Sec;

            if (sscanf(str, "%d:%d:%d", &Hour, &Min, &Sec) != 3)
                return false;
            tm.Hour = Hour;
            tm.Minute = Min;
            tm.Second = Sec;
            return true;
        }

        bool getDate(const char *str)
        {
            char Month[12];
            int Day, Year;
            uint8_t monthIndex;
           


            if (sscanf(str, "%s %d %d", Month, &Day, &Year) != 3) return false;

            for (monthIndex = 0; monthIndex < 12; monthIndex++)
            {
                if (strcmp(Month, monthName[monthIndex]) == 0)
                    break;
            }
            if (monthIndex >= 12)
                return false;
            tm.Day = Day;
            tm.Month = monthIndex + 1;
            tm.Year = CalendarYrToTm(Year);
            //tm.Year = CalendarYrToTm(Year);

            // Serial.println (str);
            // Serial.println (Year);
            // Serial.println (CalendarYrToTm(Year)); 
            // Serial.println (y2kYearToTm(CalendarYrToTm(Year)));             
            // Serial.println (tm.Year); 

            return true;
        }
#endif





// формируем меню экрана
int i, t;

char* ptrQ = (char*)" ";
        // стартовый / начальный экран
        LiquidLine welcome_line1(0, 0, "Hello Gl!"); 
        LiquidLine welcome_line2(0, 1, "v.0.0.8");
        LiquidScreen welcome_screen(welcome_line1, welcome_line2);


        LiquidLine MParam_1(0, 0,  CurrentDateTime.ptrCharDate, ptrQ, CurrentDateTime.ptrCharTime); 
        LiquidLine MParam_2(0, 1  );
        LiquidScreen MainParam(MParam_1,MParam_2); //№2


        // tStr = GetSTR_DateTime(tm,'D');  
        // LiquidLine Date_1(0, 0, "SetUp Date");
        // LiquidLine Date_2(0, 1, tStr); // Почему нельзя указать напрямую GetSTR_DateTime(&tm,'D') не понимаю. На досуге разобраться
        LiquidLine Date_1(0, 0, "SetUp Date");
        LiquidLine Date_2(0, 1, "Date Now");
        LiquidScreen mDate(Date_1,Date_2);

        LiquidLine LED_1_1(0, 0, "Set Channel Brightness");
        LiquidLine LED_1_2(0, 1, "Brightness=");
        LiquidScreen mLED_1_Screen(LED_1_1,LED_1_2);


//LiquidMenu menu(LCD_1602);
LiquidMenu menu(lcd);

void setup(void)
{       
  // Инициализация объекта I2C LCD.
  lcd.init();
  lcd.backlight();

  //time.begin();                         // инициируем RTC модуль
        // LCD_1602.init();
        // LCD_1602.backlight();
        // LCD_1602.LCDRow1 ="Hello Gl!";
        // LCD_1602.LCDRow2 ="v.0.0.7";
        // LCD_1602.LCD_Print();

    
    
        menu.init();
      //  LiquidMenu menu(lcd, welcome_screen, some_screen);
        menu.add_screen(welcome_screen);
        menu.add_screen(MainParam);        
        menu.add_screen(mDate);
        menu.add_screen(mLED_1_Screen);
        
        menu.change_screen(1);

    // ------------------------------------------------------------------------------------------------------------------    
 
 
 
 
 
    uint32_t day111;

    #ifdef DEBUG_Setup
        Serial.begin(SERIAL_BAUD);
        Serial.println("Debuging start......3.....................");
        Serial.println();
        Serial.println();
    #endif


        // настраиваем порты
        pinMode(portDC18B20PWM_1, OUTPUT);
        pinMode(portDC18B20PWM_2, OUTPUT);
        digitalWrite(portDC18B20PWM_1, LOW);
        digitalWrite(portDC18B20PWM_2, LOW);
        

       

    // устанавливаем порог температуры и инициализируем датчики температуры  
        DS18_settings.TargetTemp_D1 =DC18_MaxGoodTemp;
        DS18_settings.TargetTemp_D2 =DC18_MaxGoodTemp;
        if (DS1822_init(DS18_OneWare, DS18_settings))  
            {
               #ifdef DEBUG_Setup
                  Serial.println ("Init DC18B20 - OK");
                #endif 
                Errors[1].codeErr=0;               
            }
            else 
            {
                #ifdef DEBUG_Setup
                  Serial.println (Errors[1].TextErr);
                #endif  
                Errors[1].codeErr=1;
          //      LCD_1602.LCDRow1 = Errors[1].TextErr;        
          //      LCD_1602.LCD_Print();
            } 


    // инициализируем RTC модуль
            day111=RTC.get();   //DS1307RTC Library  ХЗ но без этой строки плата RTC не определяется
                        //Reads the current date & time as a 32 bit "time_t" number. 
                        //Zero is returned if the DS1307 is not running or does not respond.

            #ifdef SetSystemTime  // устанавливаем время в RTC = системному времени и дате в момент компиляции 
                    if (getDate(__DATE__) && getTime(__TIME__)) 
                    {
                        if (!RTC.write(tm)) //configure the RTC with this info
                        {Errors[0].codeErr=1;
        //                LCD_1602.LCDRow1 = Errors[0].TextErr;
        //                LCD_1602.LCD_Print();  
                        } 
                        else {Errors[0].codeErr=0;}
                    }
            #endif
      
            if (RTC.read(tm)) 
            {
                GetSTR_DateTime(tm,CurrentDateTime);
                #ifdef DEBUG_Setup    
                    Serial.println ("Init RTS Module - OK");    
                    Serial.println ("Время в модуле RTS:");
                    Serial.print("Дата: ");
                    Serial.print(CurrentDateTime.Date);
                    Serial.print(", Время: ");
                    Serial.print(CurrentDateTime.Time);
                    Serial.println();
                    Serial.println();
                #endif  
                Errors[0].codeErr=0; // показания считали. Ошибки нет
            }
            else 
            {
                #ifdef DEBUG_Setup                 
                    Serial.println (Errors[0].TextErr);
                #endif                     
                Errors[0].codeErr=1; 
        //        LCD_1602.LCDRow1 = Errors[0].TextErr;  
        //        LCD_1602.LCD_Print();
            }


}




void loop(void)
{
    unsigned long currentMillis = millis();
    if (currentMillis - TimerPrevMillis[0]>=TimeInterval_RTC)
            {TimerPrevMillis[0]=currentMillis;
             DateTimeUpdate();
           //  tmpI="BBBBBBB";
             menu.update();
             Serial.println(">=TimeInterval_RTC");
             Serial.print("CurrentDateTime=");
             Serial.println(CurrentDateTime.Date);
             Serial.print("CurrentDateTime=");
             Serial.println(CurrentDateTime.Time);
            }
        
    if (currentMillis - TimerPrevMillis[1]>=TimeInterval_DS18D20)
            {
                if (DS18_sensorRequest (DS18_OneWare, DS18_settings))
                    {TimerPrevMillis[1]=currentMillis;
                    menu.update();
                    Serial.print("TempRead. T1=");
                    Serial.println(DS18_settings.CurrentTemp_D1);
                    
                    Serial.print("TempRead. T2=");
                    Serial.println(DS18_settings.CurrentTemp_D2);                   
                    }
            }

    if (currentMillis - TimerPrevMillis[2]>=TimeInterval_DS18D20)
            {TimerPrevMillis[2]=currentMillis;
            lcd.noBacklight();}        



//    if      (LCD_1602.NeedOff == true)  {LCD_1602.noBacklight();}    //выключаем подсветку экрана 
//    else if (LCD_1602.NeedOff == false) {LCD_1602.backlight();}      //включаем  подсветку экрана


     

    
    switch (MyButtons.KeyPressedCode()) 
     {
    //     case 0: {menu.next_screen(); Serial.println("Key 0 pressed"); break;}
    //     case 1: {menu.change_screen(0); Serial.println("Key 1 pressed"); break;}
    //     case 2: {menu.previous_screen(); Serial.println("Key 2 pressed"); break;}
    //     case 3: {menu.change_screen(1); Serial.println("Key 3 pressed"); break;}

        case 0: {menu.change_screen(1); Serial.println("Key 0 pressed"); lcd.backlight(); TimerPrevMillis[2]=currentMillis; break;}
        case 1: {menu.change_screen(2); Serial.println("Key 1 pressed"); lcd.backlight(); TimerPrevMillis[2]=currentMillis; break;}
        case 2: {menu.change_screen(3); Serial.println("Key 2 pressed"); lcd.backlight(); TimerPrevMillis[2]=currentMillis; break;}
        case 3: {menu.change_screen(4); Serial.println("Key 3 pressed"); lcd.backlight(); TimerPrevMillis[2]=currentMillis; break;}

    };


  if (DS18_settings.D1_tempCounter == 0xFF) {   //0xFF =255 или  11111111 в бинарном. При каждои привышении граница температуры пишем 1 в новый бит если получаем 0xFF то значит темперетура превышена на протяжении заданного интервала, введено для избежания "дребезга" 
        DS18_settings.PWM_D1_Level = (DS18_settings.CurrentTemp_D1 - DS18_settings.TargetTemp_D1)*DS18_settings.PWM_StepUP;
        if (DS18_settings.PWM_D1_Level>255){DS18_settings.PWM_D1_Level=255;} //t1 -- счетчик увеличения уровня ШИМ 
        #ifdef DEBUG_Loop
            Serial.print ("D1_Current temp: ");
            Serial.print (DS18_settings.CurrentTemp_D1,5);
            Serial.print (",   D1_Target temp: ");
            Serial.print (float(DS18_settings.TargetTemp_D1),5);
            Serial.print (",  PWM level: ");
            Serial.println (float(t1),2);
        #endif
        }  
    else {DS18_settings.PWM_D1_Level=0;}

  if (DS18_settings.D2_tempCounter == 0xFF) {
        DS18_settings.PWM_D2_Level = (DS18_settings.CurrentTemp_D2 - DS18_settings.TargetTemp_D2)*DS18_settings.PWM_StepUP;
        if (DS18_settings.PWM_D2_Level>255){DS18_settings.PWM_D2_Level=255;} 
        #ifdef DEBUG_Loop
            Serial.print ("D2_Current temp: ");
            Serial.print (DS18_settings.CurrentTemp_D2,5);
            Serial.print (",   D2_Target temp: ");
            Serial.print (float(DS18_settings.TargetTemp_D2),5);
            Serial.print (",  PWM level: ");
            Serial.println (float(t2),2);
        #endif
    }  
    else {DS18_settings.PWM_D2_Level=0;}

    analogWrite(portDC18B20PWM_1, DS18_settings.PWM_D1_Level); // записываем значение ШИМ в порт   
    analogWrite(portDC18B20PWM_2, DS18_settings.PWM_D1_Level); // записываем значение ШИМ в порт

    //Обновляем на экран
  //  LCD_1602.LCD_Print();

}
