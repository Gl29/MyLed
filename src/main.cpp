#include <Arduino.h>
#include <OneWire.h>    //Библиотека 1-Wire. для цифровых датчиков температуры 
//#include <Wire.h>
//#include <RTClib.h>
#include <TimeLib.h>
#include <DS1307RTC.h>  //a basic DS1307 library that returns time as a time_t. 

// #include <EEPROM.h>  // Библиотека для работы с EEPROM 
#include <leOS.h>       // Шедуллер задач
#include <LiquidCrystal_I2C.h> //Библиотека I2C для LCD
#include <LiquidMenu.h> // Библиотека для работы с меню

#include "MyLCD.h"          //   вывод на экран 
#include "Buttons.h"
#include "MyAddFunctions.h" //сборник дополнительных функций вынесен в отдельный файл "дабы не засорять эфир" 
#include "MyDC18B20.h"      // код для работы с датчикками темперетуры

 #define MyDEBUG                        // Включает общий режим отладки
// #define DEBUG_DS18_sensorRequest   // Включает режим отладки для функции DEBUG_DS18_sensorRequest
// #define DEBUG_DS1822_init
// #define DEBUG_DS18_ReadTemp
// #define DEBUG_DS18_InitConversion
// #define DEBUG_DS18_SetDS18_resolution
// #define DEBUG_LCDTest
// #define DEBUG_Loop                      // Включает режим отладки для функции Void LOOP
#define SetSystemTime                   // Включает режим отладки для функции Void LOOP



#define SERIAL_BAUD 9600
#define DS18D20_TIME_SCAN_frequency 15000 //10000 // Время опроса датчиков температуры м.сек
#define LCD_1602BacklightOffTime    5000  // Время через которое выключается подсветка экрана м.сек
#define DC18_MaxGoodTemp 30      // Максимальная температура после превышения которой срабатывает событие (градусы цельсия)


//  PINs к которым прицеплена переферия ()
#define portOneWire 2                   // указаываем порт OneWire   протокол 1-Wire
#define portDC18B20PWM_1 5              // указаываем порт для PWM DC18B20_1  
#define portDC18B20PWM_2 6              // указаываем порт для PWM DC18B20_2
#define pordButtonsRead A0              // указаываем порт на котором будем слушать нажатие кнопок

const String errRTC = "Error read RTS";      // текст ошибки доступа к модулю RTC
const String errDC18B20 = "Error init DC18B20";  // текст ошибки инициализации датчиков темперетуры

tmElements_t tm; // переменная которая хранит время в формте TimeLib.h 
OneWire DS18_OneWare(portOneWire);   // инициируем OneWire протокол 1-Wire



// struct struct_DS18_setting           // Структура для ведения параметров датчиков температуры и сохранения данных в eeprom
//   {
//       byte D1_tempCounter;             // байт в котором буем вести историю изменения температуры, если все биты =1  значит пора включать вентилятор
//       byte D2_tempCounter;             //!!! потом надо перевести на INT + DS18_divider                                     
//       float CurrentTemp_D1;           // тут храним значение последней считанной с датчиков температуры  
//       float CurrentTemp_D2;            
//       byte TargetTemp_D1 = DC18_MaxGoodTemp;        // тут храним значение максимально допустимой температуры после которой начинаем включать вентилятор
//       byte TargetTemp_D2 = DC18_MaxGoodTemp;            
//       bool D1_Start;                  // флаги. если = истина - пора включать вентилятор
//       bool D2_Start;
//       byte PWM_StepUP = 35; //25;     // переменная хранит шаг роста ШИМ в зависимости от разницы целевой и фактической температуры
//   };

struct_DS18_setting DS18_settings;   // создаём экземпляр набора параметров

struct stuct_LedBrightness              // сртуктура хранит параметры LED каналов
{
    String channelName;                 // текстовое имя LED канала 
    byte   channalNumber;               // номер  LED канала 
    uint16_t channalBrightness ;   // значение яркости канала  микросхема TLC5940NT - 16-канальный ШИМ драйвер с 12-битным регулированием скважности (0-4096) и 6-битным (0-63) регулированием тока.
};
stuct_LedBrightness LedSettings [5] = {{"White6300_1",1,1000},{"White6300_2",2,1000},{"White4500",3,1000},{"Red",4,},{"Blue",5,1000},};


leOS myTask;                        //create a new istance of the class leOS
MyLCD  LCD_1602(0x27,16,2);           // инициируем экран //int(0x27)
Button MyButtons = Button(A0);                  // Класс отработки нажатия кнопок

void LCD_BackLight_OFF()
{ 
    LCD_1602.NeedOff = true;  // Сработал таймер, пора выключать подсветку.  
}

// byte DS18_resolution = 12;         // устанавливам точность измерения температуры на 10, оптимальным является 10битное кодирование 187,5мс
//                                    // Разрешающая способность температурного преобразователя может быть изменена пользователем и
//                                    // составляет 9, 10, 11, или 12 битов, соответствуя приращениям (дискретности измерения температуры)
//                                    // 0.5°C (93.75ms), 0.25°C(187.5ms), 0.125°C (375ms), и 0.0625°C (750ms), соответственно.
//                                    //Байт 4 памяти содержит регистр конфигурации, можно настроить конверсионную разрешающую способность DS18, используя биты R0(Бит№5) и R1(Бит№6) в этом регистре,

unsigned long starttime;
//unsigned long DS18_last_Call_Time=0; // Время последнего опроса датчиков
// uint32_t DS18_divider = 10 ^ 4;      // Делитель. UNO лучше работать с целыми числами поэтому температуру храним как целое число * DS18_divider ....


//byte DS18_readstage =0;              // переменная для процедуры DS18_sensorRequest. Если = 0 стартует конверсия (команда на считывание), если =1 - получаем температуру
//byte DS18_Sensors_addr[2][8];        // Массив. Хранит адреса датчиков. Каждое устройства типа 1-Wire обладает уникальным 64-битным ROM-адресом, который состоит из 8-битного кода, обозначающего семейство, 48-битного серийного кода и 8-битного CRC.
//byte *DS18_pAdr;                       // указатель на первый элемент массива 




// bool DS18_SetDS18_resolution(OneWire DS18_OneWare, byte DS18_Sensors_addr[8], byte DS18_resolution)
// { // Функция задаёт параметры точности датчика

//     #ifdef  MyDEBUG_DS18_SetDS18_resolution 
//          Serial.println("Func DS18_SetDS18_resolution start: ");
//     #endif
//     // задаём параметры точности датчиков

//     byte resbyte = 0x1F;
//     if (DS18_resolution == 12)
//     {
//         resbyte = 0x7F;
//     }
//     else if (DS18_resolution == 11)
//     {
//         resbyte = 0x5F;
//     }
//     else if (DS18_resolution == 10)
//     {
//         resbyte = 0x3F;
//     }

//     // Set configuration
//     DS18_OneWare.reset();
//     DS18_OneWare.select(DS18_Sensors_addr);
//     DS18_OneWare.write(0x4E);    // Write scratchpad  Эта команда позволяет устройству управления записывать 3 байта данных в память DS18 TL/TH/Configuration Register
//     DS18_OneWare.write(0);       // TL
//     DS18_OneWare.write(0);       // TH
//     DS18_OneWare.write(resbyte); // Configuration Register
//     DS18_OneWare.write(0x48);    // Copy Scratchpad Копирование ОЗУ В ПЗУ

//     // Read configuration
//     byte tData[9];
//     DS18_OneWare.reset();
//     DS18_OneWare.select(DS18_Sensors_addr);
//     DS18_OneWare.write(0xBE);

//     for (int i = 0; i < 5; i++)
//     { // нам нужно 9 байтов
//         tData[i] = DS18_OneWare.read();
//     }

//     #ifdef  DEBUG_DS18_SetDS18_resolution 
//         Serial.print("DS18_WriteConfByte=");
//         Serial.println(tData[4], HEX);
//     #endif

//     if (tData[4] !=0 ) return true; else return false;  //NULL
    
// }

// void DS18_InitConversion(OneWire DS18_OneWare, byte DS18_Sensors_addr[8])
// { // Функция отправляет всем датчикам команду начать измерение температуры
//     #ifdef  DEBUG_DS18_InitConversion 
//          Serial.println("Func DS18_InitConversion start: ");
//     #endif
    
//     DS18_OneWare.reset();

//     //  DS18_OneWare.select(DS18_Sensors_addr);
//     DS18_OneWare.write(0xCC); // skip rom  // отправляем команду сброса всем датчикам одновременно
//     DS18_OneWare.write(0x44); // start conversion

//     // DS18_OneWare.select(DS18_Sensors_addr);
//     // DS18_OneWare.write(0x44,1);         // start conversion, with parasite power on at the end
// }

// float DS18_ReadTemp(OneWire DS18_OneWare, byte DS18_Sensors_addr[8])
// { // Функция определяет тип подключённого датчика с уникальным ID который указан по адресу DS18_Sensors_addr и получает от него температуру
//     #ifdef  DEBUG_DS18_ReadTemp
//          Serial.println("Func DS18_ReadTemp start: ");
//     #endif


//     byte present = 0; 
//     int i;
//     byte data[12];
//     byte type_s;
//     int16_t celsius;

//     switch (DS18_Sensors_addr[0])
//     {
//     case 0x28:
//         //  Serial.println(F("  Chip = DS18"));
//         type_s = 0;
//         break;
//     case 0x10:
//         //   Serial.println(F("  Chip = DS18S20"));  // or old DS1820
//         type_s = 1;
//         break;

//     case 0x22:
//         // Serial.println(F("  Chip = DS1822"));
//         type_s = 0;
//         break;
//     default:
//         Serial.println(F("Error. Device is not a DS18x20 family device."));
//         Serial.print(F("DS18_Sensors_addr[0] = "));
//         Serial.println(DS18_Sensors_addr[0], HEX);
//     }

//     present = DS18_OneWare.reset();
//     DS18_OneWare.select(DS18_Sensors_addr);
//     DS18_OneWare.write(0xBE); // Read Scratchpad

//     for (i = 0; i < 9; i++)
//     { // we need 9 bytes
//         data[i] = DS18_OneWare.read();
//         //    Serial.print(data[i], HEX);
//         //    Serial.print(" ");
//     }
//     //Serial.print(" CRC=");
//     //Serial.print(OneWire::crc8(data, 8), HEX);
//     //  Serial.println();

//     // convert the data to actual temperature

//     // конвертируем данный в фактическую температуру
//     // так как результат является 16 битным целым, его надо хранить в
//     // переменной с типом данных "int16_t", которая всегда равна 16 битам,
//     // даже если мы проводим компиляцию на 32-х битном процессоре

//     int16_t raw = (data[1] << 8) + data[0];
//     //Serial.println (((float)raw/16)*DS18_divider);

//     if (type_s)
//     {
//         raw = raw << 3; // разрешение 9 бит по умолчанию
//         if (data[7] == 0x10)
//         {
//             raw = (raw & 0xFFF0) + 12 - data[6];
//         }
//     }
//     else
//     { // для DS18
//         byte cfg = (data[4] & 0x60);
//         // при маленьких значениях, малые биты не определены, давайте их обнулим
//         if (cfg == 0x00)
//         {
//             raw = raw & ~7;
//           //  Serial.println("разрешение 9 бит");
//         } // разрешение 9 бит, 93.75 мс
//         else if (cfg == 0x20)
//         {
//             raw = raw & ~3;
//           //  Serial.println("разрешение 10 бит");
//         } // разрешение 10 бит, 187.5 мс
//         else if (cfg == 0x40)
//         {
//             raw = raw & ~1;
//           //  Serial.println("разрешение 11 бит");
//         } // разрешение 11 бит, 375 мс
//         else
//         {
//            // Serial.println("разрешение 12 бит");
//         } // разрешение 11 бит, 375 мс
//     }

//     celsius = ((float)raw / 16.0) * DS18_settings.DS18_divider;

//     //   Serial.print("Temp (C): ");
//     //   Serial.println(celsius);
//     return celsius;
// }

// bool DS1822_init ()
// { // Функция инициализирует датчики температуры и записывает их уникальные номера в переменную  DS18_Sensors_addr и указатель DS18_pAdr 
//     byte DS18SensorsInitOk[2];
   
//     #ifdef  DEBUG_DS1822_init 
//          Serial.println("Func DS1822_init start: ");
//     #endif

//     DS18_settings.last_Call_Time = now();   // Время инициализации датчиков = текущему  //DS18_last_Call_Time
//     byte DS18_Sensors_tmp_addr[8]; // Массив в который DS18_OneWare.search(DS18_Sensors_addr) будет записывать адрес найденного датчика
//     byte tCount = 0;
//     while (DS18_OneWare.search(DS18_Sensors_tmp_addr))
//     {
//         for (int i = 0; i < 8; i++)
//         {
//             //DS18_Sensors_addr[tCount][i] = DS18_Sensors_tmp_addr[i]; // записываем 8 байтов уникальных кодов в память
//              DS18_settings.Sensors_addr[tCount][i] = DS18_Sensors_tmp_addr[i]; // записываем 8 байтов уникальных кодов в память
//         }
        
//        // if (DS18_SetDS18_resolution(DS18_OneWare, DS18_Sensors_tmp_addr, DS18_settings.resolution))  // задаём параметры точности датчиков    
//         if (DS18_SetResolution(DS18_OneWare, DS18_settings, DS18_Sensors_tmp_addr ))  // задаём параметры точности датчиков
       
//         {DS18SensorsInitOk[tCount] =1;} else {DS18SensorsInitOk[tCount] =0;}                // если DS18_SetDS18_resolution вернула TRUE заначит инициализация прошла успешно
//         tCount++;
//     }
//    // DS18_pAdr = &DS18_Sensors_addr[0][0]; // создаём указатель на первый элемент массива
//    DS18_settings.pAdr = &DS18_settings.Sensors_addr[0][0]; // создаём указатель на первый элемент массива

//     #ifdef  DEBUG_DS1822_init
//         for (int k = 0; k < 2; k++)
//         { // Печатаем уникальные номера датчиков
//             for (int i = 0; i < 8; i++)
//             {
//                 Serial.print(DS18_Sensors_addr[k][i], HEX);
//             }
//             Serial.println();
//         }
//         //Serial.println (*(DS18_pAdr),HEX);     //Serial.println (*(DS18_pAdr+8),HEX);   //Serial.println (*(DS18_pAdr+1),HEX);// Печать байтов памяти по указателю

//     #endif
//     if (DS18SensorsInitOk[0]==1 && DS18SensorsInitOk[1]==1) return true; else return false;

// }

// void DS18_sensorRequest ()
// { // Функция даёт датчикам команду на считывание температуры и на ёё преобразование
//     #ifdef  DEBUG_DS18_sensorRequest 
//          Serial.println("Func DS18_sensorRequest: ");
//     #endif
//     if (DS18_settings.readstage == 0)
//     {
//         #ifdef  DEBUG_DS18_sensorRequest
//             Serial.print("DS18_readstage: ");            
//             Serial.println(DS18_readstage);
//             Serial.println("Call 'ConvertCommand' ...");
//         #endif

//         DS18_InitConversion(DS18_OneWare, DS18_settings.pAdr); //DS18_pAdr);
//         DS18_settings.readstage++;
//         }
//     else
//     {
//         #ifdef  DEBUG_DS18_sensorRequest 
//             Serial.println();
//             Serial.print("DS18_readstage: ");            
//             Serial.println(DS18_readstage);
//             Serial.println("Call 'ConvertCommand' ...");

//         #endif


//         if (DS18_OneWare.read()) // если OneWare находится в статусе read (показания температуры сконвертированы и доступны для чтения)
//         {   
//             // DS18_settings.CurrentTemp_D1 = DS18_ReadTemp(DS18_OneWare, DS18_settings.pAdr) / DS18_settings.DS18_divider; //DS18_pAdr
//             // DS18_settings.CurrentTemp_D2 = DS18_ReadTemp(DS18_OneWare, (DS18_settings.pAdr+8)) / DS18_settings.DS18_divider; //DS18_pAdr

//             DS18_settings.CurrentTemp_D1 = DS18_ReadTemp(DS18_OneWare, DS18_settings, DS18_settings.pAdr) / DS18_settings.DS18_divider; //DS18_pAdr
//             DS18_settings.CurrentTemp_D2 = DS18_ReadTemp(DS18_OneWare, DS18_settings, (DS18_settings.pAdr+8)) / DS18_settings.DS18_divider; //DS18_pAdr
                         


//             if (DS18_settings.CurrentTemp_D1 > DS18_settings.TargetTemp_D1) {
//                 DS18_settings.D1_tempCounter = DS18_settings.D1_tempCounter << 1; // <<	Сдвиг влево
//                 DS18_settings.D1_tempCounter = DS18_settings.D1_tempCounter ^ 1;  //исключающее ИЛИ "^" выдает истину, если только один из операндов истинен. В противном случае получается ложь.
//                 }
//                 else
//                 {DS18_settings.D1_tempCounter = DS18_settings.D1_tempCounter << 1; // <<	Сдвиг влево первому биту будет присвоен 0
//             }

//             if (DS18_settings.CurrentTemp_D2 > DS18_settings.TargetTemp_D2) {
//                 DS18_settings.D2_tempCounter = DS18_settings.D2_tempCounter << 1; // <<	Сдвиг влево
//                 DS18_settings.D2_tempCounter = DS18_settings.D2_tempCounter ^ 1;  //исключающее ИЛИ "^" выдает истину, если только один из операндов истинен. В противном случае получается ложь.
//                 }
//                 else
//                 {DS18_settings.D2_tempCounter = DS18_settings.D2_tempCounter << 1; // <<	Сдвиг влево первому биту будет присвоен 0
//             }

//             #ifdef  DEBUG_DS18_sensorRequest 
//                 Serial.println(DS18_settings.CurrentTemp_D1, 5);
//                 Serial.println(DS18_settings.CurrentTemp_D2, 5);
    
//                 Serial.print ("DS18_settings.D1_tempCounter (byte)= ");
//                 Serial.println (DS18_settings.D1_tempCounter, BIN);

//                 Serial.print ("DS18_settings.D2_tempCounter (byte)= ");
//                 Serial.println (DS18_settings.D2_tempCounter, BIN);
//             #endif

//             DS18_settings.readstage = 0;
//             DS18_settings.last_Call_Time = now(); // сбрасываем счётчик времени опроса таймера //DS18_last_Call_Time

//          //   LCD_1602NeedOff =0; // ставим признак что надо включить подсветку
//             LCD_1602.NeedOff = false; // ставим признак что надо включить подсветку
//             myTask.restartTask(LCD_BackLight_OFF); // перезапускаем таймер выключения экрана 

//             // генерируем строку 2 LCD дисплея с данными температуры
//             LCD_1602.LCDRow2 = "t=" + String(DS18_settings.CurrentTemp_D1)  + " / " + String(DS18_settings.CurrentTemp_D2);    

//         }
//     }   
// }



void DS18B20_TempStart ()
{
 if (DS18_sensorRequest (DS18_OneWare,DS18_settings)) {
            LCD_1602.NeedOff = false; // ставим признак что надо включить подсветку
            myTask.restartTask(LCD_BackLight_OFF); // перезапускаем таймер выключения экрана 

            // генерируем строку 2 LCD дисплея с данными температуры
            LCD_1602.LCDRow2 = "t=" + String(DS18_settings.CurrentTemp_D1)  + " / " + String(DS18_settings.CurrentTemp_D2);
        } 
}


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



void setup(void)
{       

// формируем меню экрана
        // Date & Time
        String tStr;
        tStr = GetSTR_DateTime(tm,'T');  
        LiquidLine Time_1(0, 0, "SetUp Time");
        LiquidLine Time_2(0, 1, tStr);
        LiquidScreen mTime(Time_1,Time_2);

        tStr = GetSTR_DateTime(tm,'D');  
        LiquidLine Date_1(0, 0, "SetUp Date");
        LiquidLine Date_2(0, 1, tStr); // Почему нельзя указать напрямую GetSTR_DateTime(&tm,'D') не понимаю. На досуге разобраться
        LiquidScreen mDate(Date_1,Date_2);


        //LED
        LiquidLine LED_1_1(0, 0, LedSettings[0].channelName);
        LiquidLine LED_1_2(0, 1, "Brightness=",LedSettings[0].channalBrightness);
        LiquidScreen mLED_1_Screen(LED_1_1,LED_1_2);

        LiquidMenu menu(LCD_1602);



    uint32_t day111;
    
    bool parse=false;
    bool config=false;

    #ifdef MyDEBUG
        Serial.begin(SERIAL_BAUD);
        Serial.println("Debuging start......3.....................");
        Serial.println();
        Serial.println();
    #endif

    day111=RTC.get();  //DS1307RTC Library  ХЗ но без этой строки плата RTC не определяется
                    //Reads the current date & time as a 32 bit "time_t" number. 
                    //Zero is returned if the DS1307 is not running or does not respond.

    #ifdef SetSystemTime  // устанавливаем время в RTC = системному времени и дате в момент компиляции 
            if (getDate(__DATE__) && getTime(__TIME__)) 
            {
                parse = true;
                if (RTC.write(tm)) //configure the RTC with this info
                {config = true;}
            }
    #endif

        pinMode(portDC18B20PWM_1, OUTPUT);
        pinMode(portDC18B20PWM_2, OUTPUT);
        digitalWrite(portDC18B20PWM_1, LOW);
        digitalWrite(portDC18B20PWM_2, LOW);


        myTask.begin();                                                  // запускаем таймер задач
        // myTask.addTask(DS18_sensorRequest, DS18D20_TIME_SCAN_frequency); // добавляем задачу опроса температурных датчиков с
        //                                                                  //частотой DS18D20_TIME_SCAN_frequency
 
        myTask.addTask(DS18B20_TempStart, DS18D20_TIME_SCAN_frequency); // добавляем задачу опроса температурных датчиков с
                                                                         //частотой DS18D20_TIME_SCAN_frequency 
        myTask.addTask(LCD_BackLight_OFF, LCD_1602BacklightOffTime);

        

        LCD_1602.init();
        LCD_1602.backlight();
        LCD_1602.LCDRow1 ="Hello Gl!";
        LCD_1602.LCDRow2 ="v.0.0.5";
        LCD_1602.LCD_Print();

        if (DS1822_init(DS18_OneWare, DS18_settings))  //if (DS1822_init()) 
            {
               #ifdef MyDEBUG
                  Serial.println ("Init DC18B20 - OK");
                #endif bn
            }
            else 
            {
                #ifdef MyDEBUG 
                  Serial.println (errDC18B20);
                #endif  
                LCD_1602.LCDRow1 = errDC18B20;        
                LCD_1602.LCD_Print();
            } 


      
            if (RTC.read(tm)) 
            {
                #ifdef MyDEBUG    
                    Serial.println ("Init RTS Module - OK");    
                    Serial.println ("Время в модуле RTS:");
                    Serial.print("Дата: ");
                    Serial.print(tm.Day); Serial.print("/"); Serial.print(tm.Month);
                    Serial.print("/"); Serial.println(tmYearToCalendar(tm.Year));
                    Serial.print("Время: ");
                    Serial.print(tm.Hour); Serial.print(":"); Serial.print(tm.Minute); Serial.print(":");Serial.println(tm.Second);
                    Serial.println();
                    Serial.println();
                #endif  
            }
            else 
            {
                #ifdef MyDEBUG                 
                    Serial.println (errRTC);
                #endif                     
                LCD_1602.LCDRow1 =errRTC;        
                LCD_1602.LCD_Print();
                
                }
}

void loop(void)
{
   if      (LCD_1602.NeedOff == true)  {LCD_1602.noBacklight();}    //выключаем подсветку экрана 
   else if (LCD_1602.NeedOff == false) {LCD_1602.backlight();}      //включаем 

    // получаем дату/время из RTC модуля и собираем строку даты и времени для LCD дисплея
     if (RTC.read(tm)) {
        LCD_1602.LCDRow1 = GetSTR_DateTime(tm,'D'); //printDigits(tm.Day) + "." + printDigits(tm.Month) + "." + printDigits(tmYearToCalendar(tm.Year));    
        LCD_1602.LCDRow1 += " " +GetSTR_DateTime(tm,'T');} //" " +printDigits(tm.Hour) + ":"+printDigits(tm.Minute);}
     else {LCD_1602.LCDRow1 = errRTC;}

    //Выводим строки на экран
    LCD_1602.LCD_Print();


    // Serial.println(MyButtons.KeyRead());
    // Serial.println(MyButtons.PressedKey);




  if (DS18_settings.D1_tempCounter == 0xFF) {   //0xFF =255 или  11111111 в бинарном. При каждои привышении граница температуры пишем 1 в новый бит если получаем 0xFF то значит темперетура превышена на протяжении заданного интервала, введено для избежания "дребезга" 
      byte t1 = (DS18_settings.CurrentTemp_D1 - DS18_settings.TargetTemp_D1)*DS18_settings.PWM_StepUP;
      if (t1>255){t1=255;} //t1 -- счетчик увеличения уровня ШИМ 
            #ifdef DEBUG_Loop
                Serial.print ("D1_Current temp: ");
                Serial.print (DS18_settings.CurrentTemp_D1,5);
                Serial.print (",   D1_Target temp: ");
                Serial.print (float(DS18_settings.TargetTemp_D1),5);
                Serial.print (",  PWM level: ");
                Serial.println (float(t1),2);
            #endif
         analogWrite(portDC18B20PWM_1, t1); // записываем значение ШИМ в порт
         //digitalWrite(portDC18B20PWM_1, HIGH);
        }  
        else {
          //digitalWrite(portDC18B20PWM_1, LOW);
          analogWrite(portDC18B20PWM_1, 00);
        }

  if (DS18_settings.D2_tempCounter == 0xFF) {
      byte t2 = (DS18_settings.CurrentTemp_D2 - DS18_settings.TargetTemp_D2)*DS18_settings.PWM_StepUP;
      if (t2>255){t2=255;} 
            #ifdef DEBUG_Loop
                Serial.print ("D2_Current temp: ");
                Serial.print (DS18_settings.CurrentTemp_D2,5);
                Serial.print (",   D2_Target temp: ");
                Serial.print (float(DS18_settings.TargetTemp_D2),5);
                Serial.print (",  PWM level: ");
                Serial.println (float(t2),2);
            #endif
      analogWrite(portDC18B20PWM_2, t2); // записываем значение ШИМ в порт
      }  
      else {
       analogWrite(portDC18B20PWM_2, 00);
   }

}
