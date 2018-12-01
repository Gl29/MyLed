#include "LedChannel.h"



	    /**	Конструктор класса **/	
    LedChannel::LedChannel ( char *_Name, uint8_t _cBR)   
    {
        channelName         = _Name;
        targetChannelBrightness   =_cBR>MaxBrightness?MaxBrightness:_cBR;
        ptr_channelBrightness = &targetChannelBrightness;
        
    }    

	// функция рассчёта к-та яркости для канала с учётом рассвета/заката
	void LedChannel::update_PWM_Level (uint8_t CurrHour, uint8_t CurrMinute)
	{
		//static
		uint16_t currTime;                            
		//uint16_t timeIntervalFromStartTimer;      // интервал прошедший с момента старта таймера (время старата таймера минут текушее время)
		const uint16_t minInHour = 60 ;                  // минут в часе 
		uint8_t coeffBright;                        // расчётный к-т яркости (в %%)

		currTime = (minInHour*CurrHour+CurrMinute);      // текушее время = текщий час*60+текущие минуты

		for(int i=0; i<2; i++)                        // у нас два таймера для канала надо обработать оба
		{
			// рассчитываем кт яркости (плавный рассвет/закат)
			// если текущее время находится в зоне действия таймера:
			if ((currTime >=  LedTimerParam[i].hourOn*minInHour+LedTimerParam[i].minuteOn) &&   // текущее время находится в границах текущего (i) таймера 
				(currTime <=  LedTimerParam[i].hourOff*minInHour+LedTimerParam[i].minuteOff))
			{
					// для плавного увеличения яркости
					if (currTime <= LedTimerParam[i].hourOn*minInHour+LedTimerParam[i].minuteOn+LedTimerParam[i].dimmingOnDuration)
					{
						uint16_t timeIntervalFromStartTimer = currTime - (int16_t)(LedTimerParam[i].hourOn*minInHour+LedTimerParam[i].minuteOn);
					//	coeffBright = 1.0*timeIntervalFromStartTimer/LedTimerParam[i].dimmingOnDuration;
						coeffBright = 100*timeIntervalFromStartTimer/LedTimerParam[i].dimmingOnDuration;
						// coeffBright = coeffBright>1.0 ? 1.0: coeffBright;    
					//	delay (300);	
					}

					// для плавного увеличения яркости				
					else if (currTime >= (LedTimerParam[i].hourOff*minInHour+LedTimerParam[i].minuteOff - LedTimerParam[i].dimmingOffDuration))
					{
						// coeffBright = 1.0*(LedTimerParam[i].hourOff*minInHour+LedTimerParam[i].minuteOff-currTime)/LedTimerParam[i].dimmingOffDuration;
						coeffBright = 100*(LedTimerParam[i].hourOff*minInHour+LedTimerParam[i].minuteOff-currTime)/LedTimerParam[i].dimmingOffDuration;
					//	delay (300);
					}
					
					// есди не надо снижать или повышать
					else 
					{
						// coeffBright=1.0;
						coeffBright=100;						
					}        
			

				
				// uint32_t tmpI;
				// tmpI = (int32_t)(100*coeffBright);

		//		PWM_channel_level = (uint16_t)(targetChannelBrightness * coeffBright * (MaxBrightnessABS/MaxBrightness));
				// PWM_channel_level = (uint16_t)(targetChannelBrightness * tmpI * (MaxBrightnessABS/MaxBrightness)/100);

		// Serial.print ("\t");Serial.print ("\t");Serial.print ("\t");Serial.print ("\t");
		
				PWM_channel_level = 1*MaxBrightnessABS;
				// Serial.print ("\t"); Serial.print(PWM_channel_level);
				PWM_channel_level = PWM_channel_level*targetChannelBrightness;
				// Serial.print ("\t"); Serial.print(PWM_channel_level);
				PWM_channel_level = PWM_channel_level*coeffBright;		
				// Serial.print ("\t"); Serial.print(PWM_channel_level);				
				PWM_channel_level = PWM_channel_level/MaxBrightness;
				// Serial.print ("\t"); Serial.println(PWM_channel_level);
				PWM_channel_level = PWM_channel_level/100;
				// Serial.print ("\t"); Serial.println(PWM_channel_level);				
				

				// uint32_t tmpI2 = MaxBrightnessABS*targetChannelBrightness*coeffBright/MaxBrightness/100;
				// PWM_channel_level=tmpI2;

				// Serial.print ("\t");Serial.print ("\t");Serial.print ("\t");Serial.print ("\t");
				//  Serial.print("Timer NO="); Serial.print(i);    Serial.print ("\t");
				//  Serial.print("tmpI="); Serial.print(tmpI);	
				//  Serial.print("coeffBright="); Serial.print(coeffBright);					 
				//  Serial.print ("\t"); Serial.print("PWM_channel_level="); Serial.print(PWM_channel_level); 

				return;
			}   
			// else 
			// {
			// 	PWM_channel_level = 0;
			// }


			
			
			// Serial.print (F("Channel=")); Serial.print (channelName);
			// Serial.print (F("   Timer №_")); Serial.print (i);
			// Serial.print (",  PWM_channel_level="); Serial.println (PWM_channel_level);
		}


	}



    void LedChannel::SetTimer(uint8_t timerNumber, uint8_t HourOn,uint8_t MinuteOn, uint8_t  dimmingOnTime, uint8_t HourOff,uint8_t MinuteOff, uint8_t  dimmingOffTime)
    {

        LedTimerParam[timerNumber].hourOn           = HourOn>=0 && HourOn<=23                   ? HourOn : 0;
        LedTimerParam[timerNumber].minuteOn         = MinuteOn>=0 && MinuteOn<60               ? MinuteOn : 0;
        LedTimerParam[timerNumber].dimmingOnDuration    = dimmingOnTime>=0 && dimmingOnTime<60     ? dimmingOnTime : 0;

        LedTimerParam[timerNumber].hourOff           = HourOff>=0 && HourOff<=23                   ? HourOff : 0;
        LedTimerParam[timerNumber].minuteOff         = MinuteOff>=0 && MinuteOff<60               ? MinuteOff : 0;
        LedTimerParam[timerNumber].dimmingOffDuration    = dimmingOffTime>=0 && dimmingOffTime<60     ? dimmingOffTime : 0;

        // sensVal = constrain(sensVal, 10, 150);
        // числовые показания датчика ограничены диапазоном от 10 до 150

    }