#include "MyDC18B20.h"
#include <OneWire.h> 
//#include <TimeLib.h>

// // отключил в рамках замены процедурок сборки текстовой строки в модуле 
// // MyMENU void addRow (byte RowNumb, const char *Row1, float param1, const char *Row2, float param2) 
// void DS18_TempToChar(struct_DS18_setting &CurrDsSettings)
// {

//     // Serial.print("CurrDsSettings.CurrentTemp_D1=");
//     // Serial.println(CurrDsSettings.CurrentTemp_D1);
//     char t[16];
//     char t1[4];
//     strcpy(t, "t1=");
//     // Serial.print("strcpy(t, t1=). =  ");
//     // Serial.println(t);


//     dtostrf(CurrDsSettings.CurrentTemp_D1,4, 1, t1);

//     // Serial.print("dtostrf(CurrDsSettings.CurrentTemp_D1,4, 1, t1)=");
//     // Serial.println(t1);

//     strcat(t, t1);
//     strcat(t, "  t2=");
//     dtostrf(CurrDsSettings.CurrentTemp_D2,4, 1, t1);
//     strcat(t, t1);
   
//     strcpy(CurrDsSettings.CurrTempD1D2Char, t);
// }




bool DS18_SetResolution(OneWire &DS18_OneWare, struct_DS18_setting &CurrDsSettings, byte SensorAdress[8])
{ // Функция задаёт параметры точности датчика

	//byte DS18_Sensors_tmp_addr[8];

    #ifdef  MyDEBUG_DS18_SetDS18_resolution 
         Serial.println("Func DS18_SetDS18_resolution start: ");
    #endif
    // задаём параметры точности датчиков

    byte resbyte = 0x1F;
    if (CurrDsSettings.resolution == 12)  {resbyte = 0x7F;}
    else if (CurrDsSettings.resolution == 11) {resbyte = 0x5F;}
    else if (CurrDsSettings.resolution == 10) {resbyte = 0x3F;}

    // Set configuration
    DS18_OneWare.reset();
    DS18_OneWare.select(SensorAdress);
    DS18_OneWare.write(0x4E);    // Write scratchpad  Эта команда позволяет устройству управления записывать 3 байта данных в память DS18 TL/TH/Configuration Register
    DS18_OneWare.write(0);       // TL
    DS18_OneWare.write(0);       // TH
    DS18_OneWare.write(resbyte); // Configuration Register
    DS18_OneWare.write(0x48);    // Copy Scratchpad Копирование ОЗУ В ПЗУ

    // Read configuration
    byte tData[9];
    DS18_OneWare.reset();
    DS18_OneWare.select(SensorAdress);
    DS18_OneWare.write(0xBE);

    for (int i = 0; i < 5; i++)
    { // нам нужно 9 байтов
        tData[i] = DS18_OneWare.read();
    }

    #ifdef  DEBUG_DS18_SetDS18_resolution 
        Serial.print("DS18_WriteConfByte=");
        Serial.println(tData[4], HEX);
    #endif

    if (tData[4] !=0 ) return true; else return false;  
    
}

void DS18_InitConversion(OneWire &DS18_OneWare, byte DS18_Sensors_addr[8])
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

bool DS1822_init (OneWire &DS18_OneWare, struct_DS18_setting &CurrDsSettings)
{ // Функция инициализирует датчики температуры и записывает их уникальные номера в переменную  DS18_Sensors_addr и указатель DS18_pAdr 
    byte DS18SensorsInitOk[2];
   
    #ifdef  DEBUG_DS1822_init 
         Serial.println("Func DS1822_init start: ");
    #endif

    //CurrDsSettings.last_Call_Time = now();   // Время инициализации датчиков = текущему  //DS18_last_Call_Time
    byte DS18_Sensors_tmp_addr[8]; // Массив в который DS18_OneWare.search(DS18_Sensors_addr) будет записывать адрес найденного датчика
    byte tCount = 0;
    while (DS18_OneWare.search(DS18_Sensors_tmp_addr))
    {
        for (int i = 0; i < 8; i++)
        {
            //DS18_Sensors_addr[tCount][i] = DS18_Sensors_tmp_addr[i]; // записываем 8 байтов уникальных кодов в память
             CurrDsSettings.Sensors_addr[tCount][i] = DS18_Sensors_tmp_addr[i]; // записываем 8 байтов уникальных кодов в память
        }
        
       // if (DS18_SetDS18_resolution(DS18_OneWare, DS18_Sensors_tmp_addr, DS18_settings.resolution))  // задаём параметры точности датчиков    
        if (DS18_SetResolution(DS18_OneWare, CurrDsSettings, DS18_Sensors_tmp_addr ))  // задаём параметры точности датчиков
       
        {DS18SensorsInitOk[tCount] =1;} else {DS18SensorsInitOk[tCount] =0;}                // если DS18_SetDS18_resolution вернула TRUE заначит инициализация прошла успешно
        tCount++;
    }
   // DS18_pAdr = &DS18_Sensors_addr[0][0]; // создаём указатель на первый элемент массива
   CurrDsSettings.ptr_Adr = &CurrDsSettings.Sensors_addr[0][0]; // создаём указатель на первый элемент массива

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

float DS18_ReadTemp( OneWire &DS18_OneWare,   struct_DS18_setting &CurrDsSettings,   byte SensorAdress[8])
   // Функция определяет тип подключённого датчика с уникальным ID который указан по адресу DS18_Sensors_addr 
{  // и получает от него температуру
    #ifdef  DEBUG_DS18_ReadTemp
         Serial.println("Func DS18_ReadTemp start: ");
    #endif


    //byte present = 0; 
    int i;
    byte data[12];
    byte type_s;
    int16_t celsius;

    switch (SensorAdress[0])
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
        break;
     #ifdef  DEBUG_DS18_ReadTemp
        Serial.println(F("Error. Device is not a DS18x20 family device."));
        Serial.print(F("DS18_Sensors_addr[0] = "));
        Serial.println(SensorAdress[0], HEX);
     #endif   
    }

    //present = DS18_OneWare.reset();
    DS18_OneWare.reset();
    DS18_OneWare.select(SensorAdress);
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

    celsius = ((float)raw / 16.0) * CurrDsSettings.DS18_divider;

    //   Serial.print("Temp (C): ");
    //   Serial.println(celsius);
    return celsius;
}


boolean DS18_sensorRequest (OneWire &DS18_OneWare, struct_DS18_setting &CurrDsSettings)
   // именно эта функция инициирует получение температуры
{ // Функция даёт датчикам команду на считывание температуры и на ёё преобразование
    #ifdef  DEBUG_DS18_sensorRequest 
         Serial.println("Func DS18_sensorRequest: ");
    #endif
    if (CurrDsSettings.readstage == 0)
    {
        #ifdef  DEBUG_DS18_sensorRequest
            Serial.print("DS18_readstage: ");            
            Serial.println(DS18_readstage);
            Serial.println("Call 'ConvertCommand' ...");
        #endif

        DS18_InitConversion(DS18_OneWare, CurrDsSettings.ptr_Adr); //DS18_pAdr);
        CurrDsSettings.readstage++;
		return false;
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
        {   
            // DS18_settings.CurrentTemp_D1 = DS18_ReadTemp(DS18_OneWare, DS18_settings.ptr_Adr) / DS18_settings.DS18_divider; //DS18_ptr_Adr
            // DS18_settings.CurrentTemp_D2 = DS18_ReadTemp(DS18_OneWare, (DS18_settings.ptr_Adr+8)) / DS18_settings.DS18_divider; //DS18_ptr_Adr

            CurrDsSettings.CurrentTemp_D1 = DS18_ReadTemp(DS18_OneWare, CurrDsSettings, CurrDsSettings.ptr_Adr) / CurrDsSettings.DS18_divider; //DS18_pAdr
            CurrDsSettings.CurrentTemp_D2 = DS18_ReadTemp(DS18_OneWare, CurrDsSettings, (CurrDsSettings.ptr_Adr+8)) / CurrDsSettings.DS18_divider; //DS18_pAdr
            
            
            //DS18_TempToChar(CurrDsSettings); // создаём char строку в struct_DS18_setting.CurrTempD1D2Char[16]            


            if (CurrDsSettings.CurrentTemp_D1 > CurrDsSettings.TargetTemp_D1) {
                CurrDsSettings.D1_tempCounter = CurrDsSettings.D1_tempCounter << 1; // <<	Сдвиг влево
                CurrDsSettings.D1_tempCounter = CurrDsSettings.D1_tempCounter ^ 1;  //исключающее ИЛИ "^" выдает истину, если только один из операндов истинен. В противном случае получается ложь.
                }
                else
                {CurrDsSettings.D1_tempCounter = CurrDsSettings.D1_tempCounter << 1; // <<	Сдвиг влево первому биту будет присвоен 0
            }

            if (CurrDsSettings.CurrentTemp_D2 > CurrDsSettings.TargetTemp_D2) {
                CurrDsSettings.D2_tempCounter = CurrDsSettings.D2_tempCounter << 1; // <<	Сдвиг влево
                CurrDsSettings.D2_tempCounter = CurrDsSettings.D2_tempCounter ^ 1;  //исключающее ИЛИ "^" выдает истину, если только один из операндов истинен. В противном случае получается ложь.
                }
                else
                {CurrDsSettings.D2_tempCounter = CurrDsSettings.D2_tempCounter << 1; // <<	Сдвиг влево первому биту будет присвоен 0
            }

            #ifdef  DEBUG_DS18_sensorRequest 
                Serial.println(CurrDsSettings.CurrentTemp_D1, 5);
                Serial.println(CurrDsSettings.CurrentTemp_D2, 5);
    
                Serial.print ("CurrDsSettings.D1_tempCounter (byte)= ");
                Serial.println (CurrDsSettings.D1_tempCounter, BIN);

                Serial.print ("CurrDsSettings.D2_tempCounter (byte)= ");
                Serial.println (CurrDsSettings.D2_tempCounter, BIN);
            #endif

            CurrDsSettings.readstage = 0;
            //CurrDsSettings.last_Call_Time = now(); // сбрасываем счётчик времени опроса таймера //DS18_last_Call_Time

   
		return true;
        }
		else {return false;}

    }   
}


