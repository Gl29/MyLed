#ifndef BUTTONS_H
#define BUTTONS_H

#include "Arduino.h"


	class Button 
	{
		
		private:
			
//			const String ValSTR [4] = {"Right","OK","Left","Cancel"};
			int PinNumber;
			int count;
			int oldKeyValue;    // Переменная для хранения предыдущего значения состояния кнопок
			int innerKeyValue;
			int keyValue  =  0;          // Состояние покоя - Ни одна кнопка не нажата

			int GetKeyValue();
			int GetButtonNumberByValue(int);		

		public:
			enum t_PressedKey {Forward=0, Setup_Ok, Back, Cancel} ;
			Button(int);
//			String KeyPressedTxt = "";
			int8_t KeyPressedCode ();		// возвращает код нажатой кнопки или -1 если кнопка не нажата			
	};

#endif