#ifndef BUTTONS_H
#define BUTTONS_H

#include "Arduino.h"


	class Button 
	{
		private:
			const String ValSTR [5] = {"Right","OK","Left","Cancel"};
			int PinNumber;
			int count;
			int oldKeyValue;    // Переменная для хранения предыдущего значения состояния кнопок
			int innerKeyValue;
			int keyValue  =  0;          // Состояние покоя - Ни одна кнопка не нажата
		
		public:
			Button(int);

			String PressedKey = "";
			int GetKeyValue();
			int GetButtonNumberByValue(int);
		//	String KeyRead ();
			int KeyRead ();			
	};

#endif