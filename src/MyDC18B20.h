 
#include "Arduino.h"
#include <OneWire.h> 




// #define DEBUG_DS18_sensorRequest   // Включает режим отладки для функции DEBUG_DS18_sensorRequest
// #define DEBUG_DS1822_init
// #define DEBUG_DS18_ReadTemp
// #define DEBUG_DS18_InitConversion
// #define DEBUG_DS18_SetDS18_resolution


struct struct_DS18_setting           // Структура для ведения параметров датчиков температуры и сохранения данных в eeprom
  {
		byte D1_tempCounter;             // байт в котором буем вести историю изменения температуры, если все биты =1  значит пора включать вентилятор
		byte D2_tempCounter;             //!!! потом надо перевести на INT + DS18_divider                                     
		float CurrentTemp_D1;           // тут храним значение последней считанной с датчиков температуры  
		float CurrentTemp_D2;            
		byte TargetTemp_D1; // = DC18_MaxGoodTemp;        // тут храним значение максимально допустимой температуры после которой начинаем включать вентилятор
		byte TargetTemp_D2; // = DC18_MaxGoodTemp;            
		bool D1_Start;                // флаги. если = истина - пора включать вентилятор
		bool D2_Start;
		byte PWM_StepUP = 35; 	// переменная хранит шаг роста ШИМ в зависимости от разницы целевой и фактической температуры
		byte PWM_D1_Level=0;  	// уровень PWM (ШИМ) для датчика 1
		byte PWM_D2_Level=0; 	// уровень PWM (ШИМ) для датчика 2
		byte resolution = 9;	// устанавливам точность измерения температуры на 10, оптимальным является 10битное кодирование 187,5мс
									// Разрешающая способность температурного преобразователя может быть изменена пользователем и
									// составляет 9, 10, 11, или 12 битов, соответствуя приращениям (дискретности измерения температуры)
									// 0.5°C (93.75ms), 0.25°C(187.5ms), 0.125°C (375ms), и 0.0625°C (750ms), соответственно.
                                   //Байт 4 памяти содержит регистр конфигурации, можно настроить конверсионную разрешающую способность DS18, используя биты R0(Бит№5) и R1(Бит№6) в этом регистре,
		uint32_t DS18_divider = 10 ^ 4;      // Делитель. UNO лучше работать с целыми числами поэтому температуру храним как целое число * DS18_divider ....
		byte readstage =0;              // переменная для процедуры DS18_sensorRequest. Если = 0 стартует конверсия (команда на считывание), если =1 - получаем температуру
		byte Sensors_addr[2][8];        // Массив. Хранит адреса датчиков. Каждое устройства типа 1-Wire обладает уникальным 64-битным ROM-адресом, который состоит из 8-битного кода, обозначающего семейство, 48-битного серийного кода и 8-битного CRC.
		byte *pAdr;                     // указатель на первый элемент массива с адресом / адресами датчиков
		unsigned long last_Call_Time=0;	// Время последнего опроса датчиков
		//String 
  };


bool DS18_SetResolution(OneWire &DS18_OneWare, struct_DS18_setting &CurrDsSettings, byte SensorAdress[8]);
bool DS1822_init (OneWire &DS18_OneWare, struct_DS18_setting &CurrDsSettings);
float DS18_ReadTemp(OneWire &DS18_OneWare, struct_DS18_setting &CurrDsSettings, byte SensorAdress[8]);
boolean DS18_sensorRequest (OneWire &DS18_OneWare, struct_DS18_setting &CurrDsSettings);