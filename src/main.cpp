   
    //[F0h] - SEARCH ROM  (Поиск) Если Главное устройство не знает серийный номер устройства подключенного к шине, то существует возможность идентифицировать коды ROM каждого устройства подключенного к шине.
    //        Для этого необходимо использовать команду Поиск ROM [F0h] (Search). Эта команда действует как команда Чтения ROM объединенная с командой Соответствия ROM
    //[33h] - READ ROM  Эта команда может только использоваться, когда есть одно подчиненное устройство на шине. Этакоманда позволяет устройству управления шиной читать ROM подчиненного устройства (код 64 бита), не используя процедуру Поиска ROM.
    //[55h] - MATCH ROM  Команда соответствия ROM, сопровождаемая последовательностью кода ROM на 64 бита позволяет устройству управления шиной обращаться к определенному подчиненному устройству на шине.  На функциональную команду, посланную мастером; ответит только ведомый с точно соответствующим 64-разрядным кодом ROM, все другие ведомые на шине будут ожидать импульс сброса.
    //[CCh] - SKIP ROM  Пропуск ROM Главное устройство может использовать эту команду, чтобы обратиться ко всем устройствам на шине одновременно.
    //        Например, главное устройство может заставить, чтобы все DS18 (датчики температуры)на шине, начали одновременно температурные преобразования.
    //        Для этого необходимо выдать на шину команду Пропуска ROM [CCh] сопровождаемую командой Температурного преобразования [44h].

    //[ECh] - ALARM SEARCH Операция этой команды идентична команде Search ROM за исключением того, что ответят только ведомые с установленным флагом тревоги.

    //[44h] - CONVERT T Конвертировать температуру Эта команда начинает единственное температурное преобразование. После окончания преобразования данные сохраняются в 2-байтовом температурном регистре в оперативной памяти
    //        Если DS18 питается от внешнего источника питания, главное устройство может считывать состояние шины после команды Конвертирования температуры [44h].
    //        Если на шине логический «Ноль» - это значит, что DS18 выполняет температурное преобразование. Если на шине логическая «Единица» – это значит, что преобразование окончено и можно, считывать данные.
    //[4Eh] - Запись в память Эта команда позволяет устройству управления записывать 3 байта данных в память DS18. Первый байт данных записывается в регистр (TH), второй байт записывается в регистр (TL),
    //        и третий байт записывается в регистр конфигурации. Данные должны быть переданы наименьшим значащим битом вперед
    //[BEh] - Чтение памяти Эта команда позволяет Устройство управленияу читать содержание ПАМЯТИ. Передача данных начинается с наименьшего значащего бита байта 0 и продолжается до 9-ого байта (байт 8 - циклический
    //        контроль избыточности). Устройство управления может выполнить сброс, чтобы закончить чтение в любое время, если необходимо только часть данных.
    //[ECh] - Мастер-устройство может проверить состояние сигнальных флагов всех DS18 на шине, послав команду Alarm Search. Любой DS18 с установленным сигнальным флагом ответит на команду, так что мастер может точно определить, какие DS18 испытали сигнальное условие.

    //[48h] - Копирование ОЗУ В ПЗУ Эта команда копирует содержание регистров (TH, TL) и регистра конфигурации (байты 2, 3 и 4) в ПЗУ.

    //

    // байт 0 - Мл. байт температуры (50h) (85°C)
    // байт 1 - Ст. байт температуры (05h) EEPROM
    // байт 2 - Регистр TH или байт пользователя 1*
    // байт 3 - Регистр TL или байт пользователя 2*
    // байт 4 - Регистр конфигурации*
    // байт 5 - Резерв (FFh)
    // байт 6 Резерв (0Ch)
    // байт 7 Резерв (10h)
    // байт 8 CRC*
    // *Состояние при включения питания зависит от значения, сохраненного в EEPROM

    //Последовательность операций для обращения к DS18:
    //Шаг 1. Инициализация
    //Шаг 2. Команда ROM (сопровождаемая любым требуемым обменом данных) Эти команды оперируют уникальным 64-разрядным кодом ROM каждого ведомого и позволяют мастеру выбрать определенное устройство, если на шине 1-Wire присутствует несколько устройств
    //Шаг 3. Функциональная Команда DS18 (сопровождаемая любым требуемым обменом данных)

#include <Arduino.h>
#include <OneWire.h>    //Библиотека 1-Wire. для цифровых датчиков температуры 
//#include <Wire.h>       // подключаем библиотеку Wire 


#include <TimeLib.h> 
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






#define DEBUG                        // Включает общий режим отладки
//#define DEBUG_DS18_sensorRequest   // Включает режим отладки для функции DEBUG_DS18_sensorRequest
//#define DEBUG_DS1822_init
//#define DS18_ReadTemp
//#define DS18_InitConversion
//#DEBUG DS18_SetDS18_resolution
#define DEBUG_Loop                    // Включает режим отладки для функции Void LOOP

#define SERIAL_BAUD 9600
#define DS18D20_TIME_SCAN_frequency 5000 //10000 // Время опроса датчиков температуры м.сек


//  PINs к которым прицеплена переферия ()
#define portOneWire 2                   // указаываем порт OneWire   протокол 1-Wire
#define portDC28B20PWM_1 5              // указаываем порт для PWM DC18B20_1  
#define portDC28B20PWM_2 6              // указаываем порт для PWM DC18B20_2






byte DS18_resolution = 12;         // устанавливам точность измерения температуры на 10, оптимальным является 10битное кодирование 187,5мс
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
      byte TargetTemp_D1 = 30;        // тут храним значение максимально допустимой температуры после которой начинаем включать вентилятор
      byte TargetTemp_D2 = 30;            
      bool D1_Start;                  // флаги. если = истина - пора включать вентилятор
      bool D2_Start;
      byte PWM_StepUP = 35; //25;     // переменная хранит шаг роста ШИМ в зависимости от разницы целевой и фактической температуры

  };

struct_DS18_setting DS18_settings;  // создаём экземпляр набора параметров
OneWire DS18_OneWare(portOneWire);  // инициируем OneWire протокол 1-Wire

#include <LCD.h>
MyLCD LCD_1602(0x27, 16, 2); // инициируем экран

leOS myTask;                        //create a new istance of the class leOS

unsigned long TimerTik_Count=0;

void DS18_SetDS18_resolution(OneWire DS18_OneWare, byte DS18_Sensors_addr[8], byte DS18_resolution)
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

void DS1822_init ()
{ // Функция инициализирует датчики температуры и записывает их уникальные номера в переменную  DS18_Sensors_addr и указатель DS18_pAdr 
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
        DS18_SetDS18_resolution(DS18_OneWare, DS18_Sensors_tmp_addr, DS18_resolution); // задаём параметры точности датчиков
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
          //  LCD_Print(DS18_settings.CurrentTemp_D1,DS18_settings.CurrentTemp_D2); // выводим температуры на экран
          //  LCD_1602.LCD_Print_temp(DS18_settings.CurrentTemp_D1,DS18_settings.CurrentTemp_D2);

            LCD_1602.LCD_Print ("S1.temp=",DS18_settings.CurrentTemp_D1,"S1.temp=", DS18_settings.CurrentTemp_D2);


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
        }
    }   
}

void setup(void)
{
#ifdef DEBUG
    Serial.begin(SERIAL_BAUD);
    Serial.println("Debuging start......4.....................");
#endif

    myTask.begin(); // запускаем таймер задач

    LCD_1602.init();
    LCD_1602.backlight();
    LCD_1602.LCD_Print("Hello Gl!", -99, "v.0.0.4", -99);

    pinMode(portDC28B20PWM_1, OUTPUT);
    pinMode(portDC28B20PWM_2, OUTPUT);
    digitalWrite(portDC28B20PWM_1, LOW);
    digitalWrite(portDC28B20PWM_2, LOW);

    DS1822_init(); // инициируем датчики температуры
    Serial.println("Before: myTask.addTask");
    myTask.addTask(DS18_sensorRequest, DS18D20_TIME_SCAN_frequency); // добавляем задачу опроса температурных датчиков с
                                                                     //частотой DS18D20_TIME_SCAN_frequency
    Serial.println("After: myTask.addTask");
}

void loop(void)
{
    Serial.println("loop(void");

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
         analogWrite(portDC28B20PWM_1, t1); // записываем значение ШИМ в порт
         //digitalWrite(portDC28B20PWM_1, HIGH);
        }  
        else {
          //digitalWrite(portDC28B20PWM_1, LOW);
          analogWrite(portDC28B20PWM_1, 00);
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
      analogWrite(portDC28B20PWM_2, t2); // записываем значение ШИМ в порт
      //digitalWrite(portDC28B20PWM_1, HIGH);
      }  
      else {
          //digitalWrite(portDC28B20PWM_2, LOW);
          analogWrite(portDC28B20PWM_2, 00);
   }

}
