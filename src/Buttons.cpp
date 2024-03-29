#include "Buttons.h"
//#define DEBUG_Butoons 


Button::Button(int _PinNumber):PinNumber(_PinNumber) {};


int8_t Button::KeyPressedCode(const uint32_t &currentMillis)			// Главная функция (вход)
{

	if (currentMillis  - LastPresstime < KeyNotReactInterval &&
		currentMillis  - LastPresstime < KeyTimeForStiky			)  	// если не прошло заданное KeyNotReactInterval время с момента нажатия кнопки 
																		// и не пришло время "залипания"
	{
		return -1;
	}

	int t1 = analogRead(PinNumber);
	
	// Serial.print(F("analogRead="));
	// Serial.println(t1);

	currKeyValue = GetButtonNumberByValue(t1); 			// Проверяем значение, не нажата ли кнопка
	if (currKeyValue == -1) 							// -1 Кнопка не нажата
			{
				StickyKey=false;
				prevKeyValue=-1;
				currKeyValue=-1;
				return -1;

			}
	else 												// Кнопка была нажата
	{
		LastPresstime = currentMillis;
	
		if (currKeyValue == prevKeyValue && currentMillis  - LastPresstime > KeyTimeForStiky) 				
				// значение нажатой сейчас кнопки = значению кнопки нажатой ранее	(подозреваем что кнопка "зажата")
			{	
				return currKeyValue;
				// Serial.print(F("currKeyValue=")); Serial.println(currKeyValue,0);				
			}
		else if (prevKeyValue==-1 || currKeyValue != prevKeyValue)
			{	
				// Serial.print(F("currKeyValue=")); Serial.println(currKeyValue,0);
				return currKeyValue;
			}	
		else 
		{
			// Serial.print(F("currKeyValue=-1"));
			return -1;
		}		
					// #ifdef DEBUG_Butoons
					// 	Serial.println ("countNotReacting >= KeyNotReadInterval && !StickyKey");
					// 	Serial.print ("countNotReacting= "); Serial.println (countNotReacting);
					// 	Serial.print ("KeyNotReadInterval= "); Serial.println (KeyNotReadInterval);
					// 	Serial.print ("StickyKey= "); Serial.println (StickyKey);
					// 	Serial.print ("prevKeyValue= "); Serial.println (prevKeyValue);
					// 	Serial.print ("currKeyValue= "); Serial.println (currKeyValue);				
					// //	Serial.print ("tmpCountI= "); Serial.println (tmpCountI);
					// #endif
	}		
}



int Button::GetButtonNumberByValue(int value) {   	// функция по преобразованию кода нажатой кнопки в её номер
//  Со временеи значения value возвращаемые кнопкой начинают "плавать"
// 	Если кнопки распознаются не  правильно в первую очередь Необходимо проверить корреткность 
//	значений в int values[4]

  int values[4] = { 412, 510, 673, 989};  			// Величина ожидаемых значений от АЦП для разных кнопок
  int error     = ToleranceLevel;                     			// Величина отклонения от значений - погрешность
  for (int i = 0; i <= 3; i++) 
  {
    // Если значение в заданном диапазоне values[i]+/-error - считаем, что кнопка определена
    if (value <= values[i] + error && value >= values[i] - error){
		 Serial.println("GetButtonNumberByValue......");
		 Serial.print("i= "); Serial.print(i); Serial.print("     value= "); Serial.println(value); 
		 Serial.println();
		// Serial.println();
	//	tmpCountI++; // удалить как всё настрою
		return i;
     }
  }
  return -1;    
}



