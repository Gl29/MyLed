#include <Arduino.h>
#include <OneWire.h>    //Библиотека 1-Wire. для цифровых датчиков температуры 
//#include <Wire.h>
//#include <RTClib.h>
#include <TimeLib.h>
#include <DS1307RTC.h>  //a basic DS1307 library that returns time as a time_t. 
                        // DS1307RTC allows you to access real time clock (RTC) chips compatible with the DS1307. 
                        // It is intended to be used with the Time library.


// #include <EEPROM.h>  // Библиотека для работы с EEPROM 
#include <leOS.h>       // Шедуллер задач
                        // Ядро leOS обеспечивает диспетчеризацию вызовов пользовательских функций согласно заданным временным интервалам. 
                        // Для этого leOS использует аппаратный Timer2 для отсчета интервалов между вызовами пользовательских функций, 
                        // поэтому теряется доступ к функциям ШИМ на пинах D3 и D11. В ядре используется 64-битный счетчик, 
                        // так что переполнение планировщика произойдет только через 584 942 417 лет. 


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


#include "MyLCD.h"


 #define DEBUG                        // Включает общий режим отладки


// #define DEBUG_DS18_sensorRequest   // Включает режим отладки для функции DEBUG_DS18_sensorRequest
// #define DEBUG_DS1822_init
// #define DEBUG_DS18_ReadTemp
// #define DEBUG_DS18_InitConversion
// #define DEBUG_DS18_SetDS18_resolution
// #define DEBUG_LCDTest
#define DEBUG_Loop                      // Включает режим отладки для функции Void LOOP
#define SetSystemTime                   // Включает режим отладки для функции Void LOOP


#define SERIAL_BAUD 9600
#define DS18D20_TIME_SCAN_frequency 15000 //10000 // Время опроса датчиков температуры м.сек
#define LCD_1602BacklightOffTime    5000  // Время через которое выключается подсветка экрана м.сек

#define DC18_MaxGoodTemp 30      // Максимальная температура после превышения которой срабатывает событие (градусы цельсия)


//  PINs к которым прицеплена переферия ()
#define portOneWire 2                   // указаываем порт OneWire   протокол 1-Wire
#define portDC18B20PWM_1 5              // указаываем порт для PWM DC18B20_1  
#define portDC18B20PWM_2 6              // указаываем порт для PWM DC18B20_2

const String errRTC = "Error read RTS";      // текст ошибки доступа к модулю RTC
const String errDC18B20 = "Error init DC18B20";  // текст ошибки инициализации датчиков темперетуры

//uint32_t tmp1=0; //переменная для теста. Удалить

leOS myTask;                        //create a new istance of the class leOS

// class MyLCD : public LiquidCrystal_I2C
// {
//   public:
//     String LCDRow1; // перемення в которой записываем верхнюю (1) строку LCD дисплея
//     String LCDRow2; // перемення в которой записываем нижнюю (2) строку LCD дисплея


//     MyLCD(uint8_t lcd_Addr,uint8_t lcd_cols,uint8_t lcd_rows) : LiquidCrystal_I2C ( lcd_Addr, lcd_cols, lcd_rows)   // MyLCD передается в конструктор с параметром класса FirstClass
//     {}
 
//     void LCD_Print() // Функция которая выводит на LCD значение температуры
//     {   
//         LiquidCrystal_I2C::setCursor(0, 0); // Устанавливаем курсор в начало 1 строки
//         LiquidCrystal_I2C::print(LCDRow1);
//         LiquidCrystal_I2C::setCursor(0, 1);
//         LiquidCrystal_I2C::print(LCDRow2);
     
//       #ifdef DEBUG_LCDTest
//         Serial.print ("LCDRow1=");
//         Serial.println (LCDRow1);
//         Serial.print ("LCDRow2=");
//         Serial.println (LCDRow2);
//       #endif
//     }



// };

MyLCD  LCD_1602(0x27,16,2);           // инициируем экран //int(0x27)
byte LCD_1602NeedOff=0;               // Флаг необходимости выключить подсветку 



void LCD_BackLight_OFF()
{ 
    LCD_1602NeedOff =1;     // Сработал таймер, пора выключать подсветку.  
}

byte DS18_resolution = 12;           // устанавливам точность измерения температуры на 10, оптимальным является 10битное кодирование 187,5мс
                                   // Разрешающая способность температурного преобразователя может быть изменена пользователем и
                                   // составляет 9, 10, 11, или 12 битов, соответствуя приращениям (дискретности измерения температуры)
                                   // 0.5°C (93.75ms), 0.25°C(187.5ms), 0.125°C (375ms), и 0.0625°C (750ms), соответственно.
                                   //Байт 4 памяти содержит регистр конфигурации, можно настроить конверсионную разрешающую способность DS18, используя биты R0(Бит№5) и R1(Бит№6) в этом регистре,

unsigned long starttime;
unsigned long DS18_last_Call_Time=0; // Время последнего опроса датчиков
uint32_t DS18_divider = 10 ^ 4;      // Делитель. UNO лучше работать с целыми числами поэтому температуру храним как целое число * DS18_divider ....


byte DS18_readstage =0;              // переменная для процедуры DS18_sensorRequest. Если = 0 стартует конверсия (команда на считывание), если =1 - получаем температуру
byte DS18_Sensors_addr[2][8];        // Массив. Хранит адреса датчиков. Каждое устройства типа 1-Wire обладает уникальным 64-битным ROM-адресом, который состоит из 8-битного кода, обозначающего семейство, 48-битного серийного кода и 8-битного CRC.
byte *DS18_pAdr;                     // указатель на первый элемент массива 

struct struct_DS18_setting           // Структура для ведения параметров и сохранения данных в eeprom
  {
      byte D1_tempCounter;             // байт в котором буем вести историю изменения температуры, если все биты =1 
                                       // значит пора включать вентилятор
      byte D2_tempCounter;          
                                       //!!! потом надо перевести на INT + DS18_divider
      float CurrentTemp_D1;           // тут храним значение последней считанной с датчиков температуры  
      float CurrentTemp_D2;            
      byte TargetTemp_D1 = DC18_MaxGoodTemp;        // тут храним значение максимально допустимой температуры после которой начинаем включать вентилятор
      byte TargetTemp_D2 = DC18_MaxGoodTemp;            
      bool D1_Start;                  // флаги. если = истина - пора включать вентилятор
      bool D2_Start;
      byte PWM_StepUP = 35; //25;     // переменная хранит шаг роста ШИМ в зависимости от разницы целевой и фактической температуры

  };

struct_DS18_setting DS18_settings;   // создаём экземпляр набора параметров
OneWire DS18_OneWare(portOneWire);   // инициируем OneWire протокол 1-Wire




//unsigned long TimerTik_Count=0;  //прописать комментарий
tmElements_t tm; // переменная которая хранит время в формте TimeLib.h

bool DS18_SetDS18_resolution(OneWire DS18_OneWare, byte DS18_Sensors_addr[8], byte DS18_resolution)
{ // Функция задаёт параметры точности датчика

    #ifdef  DEBUG_DS18_SetDS18_resolution 
         Serial.println("Func DS18_SetDS18_resolution start: ");
    #endif
    // задаём параметры точности датчиков

    byte resbyte = 0x1F;
    if (DS18_resolution == 12)
    {
        resbyte = 0x7F;
    }
    else if (DS18_resolution == 11)
    {
        resbyte = 0x5F;
    }
    else if (DS18_resolution == 10)
    {
        resbyte = 0x3F;
    }

    // Set configuration
    DS18_OneWare.reset();
    DS18_OneWare.select(DS18_Sensors_addr);
    DS18_OneWare.write(0x4E);    // Write scratchpad  Эта команда позволяет устройству управления записывать 3 байта данных в память DS18 TL/TH/Configuration Register
    DS18_OneWare.write(0);       // TL
    DS18_OneWare.write(0);       // TH
    DS18_OneWare.write(resbyte); // Configuration Register
    DS18_OneWare.write(0x48);    // Copy Scratchpad Копирование ОЗУ В ПЗУ

    // Read configuration
    byte tData[9];
    DS18_OneWare.reset();
    DS18_OneWare.select(DS18_Sensors_addr);
    DS18_OneWare.write(0xBE);

    for (int i = 0; i < 5; i++)
    { // нам нужно 9 байтов
        tData[i] = DS18_OneWare.read();
    }

    #ifdef  DEBUG_DS18_SetDS18_resolution 
        Serial.print("DS18_WriteConfByte=");
        Serial.println(tData[4], HEX);
    #endif

    if (tData[4] !=0 ) return true; else return false;  //NULL
    
}

void DS18_InitConversion(OneWire DS18_OneWare, byte DS18_Sensors_addr[8])
{ // Функция отправляет всем датчикам команду начать измерение температуры
    #ifdef  DEBUG_DS18_InitConversion 
         Serial.println("Func DS18_InitConversion start: ");
    #endif
    
    DS18_OneWare.reset();

    //  DS18_OneWare.select(DS18_Sensors_addr);
    DS18_OneWare.write(0xCC); // skip rom  // отправляем команду сброса всем датчикам одновременно
    DS18_OneWare.write(0x44); // start conversion

    // DS18_OneWare.select(DS18_Sensors_addr);
    // DS18_OneWare.write(0x44,1);         // start conversion, with parasite power on at the end
}

float DS18_ReadTemp(OneWire DS18_OneWare, byte DS18_Sensors_addr[8])
{ // Функция определяет тип подключённого датчика с уникальным ID который указан по адресу DS18_Sensors_addr и получает от него температуру
    #ifdef  DEBUG_DS18_ReadTemp
         Serial.println("Func DS18_ReadTemp start: ");
    #endif


    byte present = 0; 
    int i;
    byte data[12];
    byte type_s;
    int16_t celsius;

    switch (DS18_Sensors_addr[0])
    {
    case 0x28:
        //  Serial.println(F("  Chip = DS18"));
        type_s = 0;
        break;
    case 0x10:
        //   Serial.println(F("  Chip = DS18S20"));  // or old DS1820
        type_s = 1;
        break;

    case 0x22:
        // Serial.println(F("  Chip = DS1822"));
        type_s = 0;
        break;
    default:
        Serial.println(F("Error. Device is not a DS18x20 family device."));
        Serial.print(F("DS18_Sensors_addr[0] = "));
        Serial.println(DS18_Sensors_addr[0], HEX);
    }

    present = DS18_OneWare.reset();
    DS18_OneWare.select(DS18_Sensors_addr);
    DS18_OneWare.write(0xBE); // Read Scratchpad

    for (i = 0; i < 9; i++)
    { // we need 9 bytes
        data[i] = DS18_OneWare.read();
        //    Serial.print(data[i], HEX);
        //    Serial.print(" ");
    }
    //Serial.print(" CRC=");
    //Serial.print(OneWire::crc8(data, 8), HEX);
    //  Serial.println();

    // convert the data to actual temperature

    // конвертируем данный в фактическую температуру
    // так как результат является 16 битным целым, его надо хранить в
    // переменной с типом данных "int16_t", которая всегда равна 16 битам,
    // даже если мы проводим компиляцию на 32-х битном процессоре

    int16_t raw = (data[1] << 8) + data[0];
    //Serial.println (((float)raw/16)*DS18_divider);

    if (type_s)
    {
        raw = raw << 3; // разрешение 9 бит по умолчанию
        if (data[7] == 0x10)
        {
            raw = (raw & 0xFFF0) + 12 - data[6];
        }
    }
    else
    { // для DS18
        byte cfg = (data[4] & 0x60);
        // при маленьких значениях, малые биты не определены, давайте их обнулим
        if (cfg == 0x00)
        {
            raw = raw & ~7;
          //  Serial.println("разрешение 9 бит");
        } // разрешение 9 бит, 93.75 мс
        else if (cfg == 0x20)
        {
            raw = raw & ~3;
          //  Serial.println("разрешение 10 бит");
        } // разрешение 10 бит, 187.5 мс
        else if (cfg == 0x40)
        {
            raw = raw & ~1;
          //  Serial.println("разрешение 11 бит");
        } // разрешение 11 бит, 375 мс
        else
        {
           // Serial.println("разрешение 12 бит");
        } // разрешение 11 бит, 375 мс
    }

    celsius = ((float)raw / 16.0) * DS18_divider;

    //   Serial.print("Temp (C): ");
    //   Serial.println(celsius);
    return celsius;
}

bool DS1822_init ()
{ // Функция инициализирует датчики температуры и записывает их уникальные номера в переменную  DS18_Sensors_addr и указатель DS18_pAdr 
    byte DS18SensorsInitOk[2];
   
    #ifdef  DEBUG_DS1822_init 
         Serial.println("Func DS1822_init start: ");
    #endif

    DS18_last_Call_Time = now();   // Время инициализации датчиков = текущему
    byte DS18_Sensors_tmp_addr[8]; // Массив в который DS18_OneWare.search(DS18_Sensors_addr) будет записывать адрес найденного датчика
    byte tCount = 0;
    while (DS18_OneWare.search(DS18_Sensors_tmp_addr))
    {
        for (int i = 0; i < 8; i++)
        {
            DS18_Sensors_addr[tCount][i] = DS18_Sensors_tmp_addr[i]; // записываем 8 байтов уникальных кодов в память
        }
        
        if (DS18_SetDS18_resolution(DS18_OneWare, DS18_Sensors_tmp_addr, DS18_resolution))  // задаём параметры точности датчиков
        {DS18SensorsInitOk[tCount] =1;} else {DS18SensorsInitOk[tCount] =0;}                // если DS18_SetDS18_resolution вернула TRUE заначит инициализация прошла успешно
        tCount++;
    }
    DS18_pAdr = &DS18_Sensors_addr[0][0]; // создаём указатель на первый элемент массива


    #ifdef  DEBUG_DS1822_init
        for (int k = 0; k < 2; k++)
        { // Печатаем уникальные номера датчиков
            for (int i = 0; i < 8; i++)
            {
                Serial.print(DS18_Sensors_addr[k][i], HEX);
            }
            Serial.println();
        }
        //Serial.println (*(DS18_pAdr),HEX);     //Serial.println (*(DS18_pAdr+8),HEX);   //Serial.println (*(DS18_pAdr+1),HEX);// Печать байтов памяти по указателю

    #endif
    if (DS18SensorsInitOk[0]==1 && DS18SensorsInitOk[1]==1) return true; else return false;

}

void DS18_sensorRequest ()
{ // Функция даёт датчикам команду на считывание температуры и на ёё преобразование
    #ifdef  DEBUG_DS18_sensorRequest 
         Serial.println("Func DS18_sensorRequest: ");
    #endif
    if (DS18_readstage == 0)
    {
        #ifdef  DEBUG_DS18_sensorRequest
            Serial.print("DS18_readstage: ");            
            Serial.println(DS18_readstage);
            Serial.println("Call 'ConvertCommand' ...");
        #endif

        DS18_InitConversion(DS18_OneWare, DS18_pAdr);
        DS18_readstage++;
        }
    else
    {
        #ifdef  DEBUG_DS18_sensorRequest 
            Serial.println();
            Serial.print("DS18_readstage: ");            
            Serial.println(DS18_readstage);
            Serial.println("Call 'ConvertCommand' ...");

        #endif


        if (DS18_OneWare.read()) // если OneWare находится в статусе read (показания температуры сконвертированы и доступны для чтения)
        {   DS18_settings.CurrentTemp_D1 = DS18_ReadTemp(DS18_OneWare, DS18_pAdr) / DS18_divider;
            DS18_settings.CurrentTemp_D2 = DS18_ReadTemp(DS18_OneWare, (DS18_pAdr+8)) / DS18_divider;
            
            if (DS18_settings.CurrentTemp_D1 > DS18_settings.TargetTemp_D1) {
                DS18_settings.D1_tempCounter = DS18_settings.D1_tempCounter << 1; // <<	Сдвиг влево
                DS18_settings.D1_tempCounter = DS18_settings.D1_tempCounter ^ 1;  //исключающее ИЛИ "^" выдает истину, если только один из операндов истинен. В противном случае получается ложь.
                }
                else
                {DS18_settings.D1_tempCounter = DS18_settings.D1_tempCounter << 1; // <<	Сдвиг влево первому биту будет присвоен 0
            }

            if (DS18_settings.CurrentTemp_D2 > DS18_settings.TargetTemp_D2) {
                DS18_settings.D2_tempCounter = DS18_settings.D2_tempCounter << 1; // <<	Сдвиг влево
                DS18_settings.D2_tempCounter = DS18_settings.D2_tempCounter ^ 1;  //исключающее ИЛИ "^" выдает истину, если только один из операндов истинен. В противном случае получается ложь.
                }
                else
                {DS18_settings.D2_tempCounter = DS18_settings.D2_tempCounter << 1; // <<	Сдвиг влево первому биту будет присвоен 0
            }

            #ifdef  DEBUG_DS18_sensorRequest 
                Serial.println(DS18_settings.CurrentTemp_D1, 5);
                Serial.println(DS18_settings.CurrentTemp_D2, 5);
    
                Serial.print ("DS18_settings.D1_tempCounter (byte)= ");
                Serial.println (DS18_settings.D1_tempCounter, BIN);

                Serial.print ("DS18_settings.D2_tempCounter (byte)= ");
                Serial.println (DS18_settings.D2_tempCounter, BIN);
            #endif

            DS18_readstage = 0;
            DS18_last_Call_Time = now(); // сбрасываем счётчик времени опроса таймера

            LCD_1602NeedOff =0; // ставим признак что надо включить подсветку
            myTask.restartTask(LCD_BackLight_OFF); // перезапускаем таймер выключения экрана 

            // генерируем строку 2 LCD дисплея с данными температуры
            LCD_1602.LCDRow2 = "t=" + String(DS18_settings.CurrentTemp_D1)  + " / " + String(DS18_settings.CurrentTemp_D2);    

        }
    }   
}


String printDigits(int digits) { // функция превращает число в текстовую строку, если число из одной цифры то дописывает ведущий ноль
  String t;
  if(digits < 10) 
    {t="0"+ String(digits);}
    else {t=digits;}
    return t;
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
    uint32_t day111;
   
    bool parse=false;
    bool config=false;

    #ifdef DEBUG
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
        myTask.addTask(DS18_sensorRequest, DS18D20_TIME_SCAN_frequency); // добавляем задачу опроса температурных датчиков с
                                                                         //частотой DS18D20_TIME_SCAN_frequency
        myTask.addTask(LCD_BackLight_OFF, LCD_1602BacklightOffTime);


        LCD_1602.init();
        LCD_1602.backlight();
        LCD_1602.LCDRow1 ="Hello Gl!";
        LCD_1602.LCDRow2 ="v.0.0.5";
        LCD_1602.LCD_Print();

        if (DS1822_init()) 
            {
               #ifdef DEBUG
                  Serial.println ("Init DC18B20 - OK");
                #endif
            }
            else 
            {
                #ifdef DEBUG 
                  Serial.println (errDC18B20);
                #endif  
                LCD_1602.LCDRow1 = errDC18B20;        
                LCD_1602.LCD_Print();
            } 


      
            if (RTC.read(tm)) 
            {
                #ifdef DEBUG    
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
                #ifdef DEBUG                 
                    Serial.println (errRTC);
                #endif                     
                LCD_1602.LCDRow1 =errRTC;        
                LCD_1602.LCD_Print();
                
                }
}

void loop(void)
{
   if  (LCD_1602NeedOff == 1)
        {LCD_1602.noBacklight();}   //включаем / выключаем подсветку экрана 
   else if (LCD_1602NeedOff == 0){LCD_1602.backlight();}


    // получаем дату/время из RTC модуля и собираем строку даты и времени для LCD дисплея
     if (RTC.read(tm)) {
        LCD_1602.LCDRow1 = printDigits(tm.Day) + "." + printDigits(tm.Month) + "." + printDigits(tmYearToCalendar(tm.Year));    
        LCD_1602.LCDRow1 += " " +printDigits(tm.Hour) + ":"+printDigits(tm.Minute);}
     else {LCD_1602.LCDRow1 = "Error read RTS";}


    //Выводим строки на экран
    LCD_1602.LCD_Print();

  if (DS18_settings.D1_tempCounter == 0xFF) {   //0xFF =255 или  11111111 в бинарном. При каждои привышении граница температуры пишем 1 в новый быит если получаем 0xFF то значит темперетура превышена на протяжении заданного интервала, введено для избежания "дребезга" 
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
