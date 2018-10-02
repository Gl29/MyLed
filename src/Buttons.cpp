#include "Buttons.h"
//#define DEBUG_Butoons 

Button::Button(int _PinNumber) {PinNumber=_PinNumber;};

int Button::GetKeyValue() {         // Функция устраняющая дребезг
	int t1 = analogRead(PinNumber);
	int actualKeyValue = GetButtonNumberByValue(t1);  // Преобразовываем его в номер кнопки, тем самым убирая погрешность
  
  	#ifdef DEBUG_Butoons
		Serial.println();Serial.println();
		Serial.print ("PinNumber=");Serial.println (PinNumber);
		Serial.print ("analogRead(PinNumber)=");Serial.println (t1);
		Serial.print ("actualKeyValue=");Serial.println (actualKeyValue);
	#endif

	if (innerKeyValue != actualKeyValue) {  // Пришло значение отличное от предыдущего
		count = 0;                            		// Все обнуляем и начинаем считать заново
		innerKeyValue = actualKeyValue;       		// Запоминаем новое значение
	}
	else {count += 1;}                          	// Увеличиваем счетчик
	

	if ((count >= 10) && (actualKeyValue != oldKeyValue)) {
		oldKeyValue = actualKeyValue;         		// Запоминаем новое значение
	}
	return    oldKeyValue;
}


int Button::GetButtonNumberByValue(int value) {   // функция по преобразованию кода нажатой кнопки в её номер
 
  int values[4] = {420, 526, 698, 1023};  // Величина ожидаемых значений от АЦП для разных кнопок
  int error     = 50;                     // Величина отклонения от значений - погрешность
  for (int i = 0; i <= 3; i++) {
    // Если значение в заданном диапазоне values[i]+/-error - считаем, что кнопка определена
    if (value <= values[i] + error && value >= values[i] - error){
        return i;
     }
  }
  return -1;                              // Значение не принадлежит заданному диапазону
}


int Button::KeyRead(){
  int newKeyValue = GetKeyValue(); // Получаем актуальное состояние кнопок с коррекцией дребезга

  if (keyValue != newKeyValue) {  // Если новое значение не совпадает со старым - реагируем на него
    keyValue = newKeyValue;       // Актуализируем переменную хранения состояния
    if (keyValue > -1) {           // Если значение больше 0, значит кнопка нажата
      Serial.println("Key pressed: " + String(keyValue));
      Serial.println("Key pressed: " + ValSTR[keyValue]);
    // return ValSTR[keyValue];
	PressedKey = ValSTR[keyValue]+"\0";
	return keyValue;
    }
  }
}
