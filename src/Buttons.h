#ifndef BUTTONS_H
#define BUTTONS_H

#include "Arduino.h"
#define KeyNotReactInterval 500 	// время с последнего нажатия кнопки в рамках которого мы не реагируем на новое нажатие 	
#define KeyTimeForStiky	 	3000 	// Количество мс нажатия одной и той же кнопки после которого считаем что есть залипание 
#define ToleranceLevel 		50		// чуствительность (граница диапазона)  для Button::GetButtonNumberByValue

	class Button 
	{
		
	private:
		int 			PinNumber;
		unsigned long 	LastPresstime; 						// Переменная хранит время работы в ms (время c последнего идентифицированного нажития кнопки )
		int				currKeyValue 			= -1;		// Код нажатой кнопки, или -1 если не нажата
		int				prevKeyValue 			= -1;		// Код нажатой кнопки, или -1 если не нажата
		bool 			StickyKey = false;		// признак удерживания кнопки (если TRUE то значит пользователь держит кнопку)	

		int count;
		int GetKeyValue();
		int GetButtonNumberByValue(int);		

	public:
			enum t_PressedKey {Forward=0, Setup_Ok, Back, Cancel} ;
			Button(int);
			int8_t KeyPressedCode ();		// возвращает код нажатой кнопки или -1 если кнопка не нажата			
	};

#endif