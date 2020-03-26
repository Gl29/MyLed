#include "LedChannel.h"



	    /**	Конструктор класса **/	
    LedChannel::LedChannel ( char *_Name, uint8_t _cBR)   
    {
        channelName         		= _Name;
        targetChnlBright_percent	=_cBR>MaxBrightness?MaxBrightness:_cBR;
        ptr_channelBrightness		= &targetChnlBright_percent;
        
    }    

	// увеличиваем яркость на 1%
    void LedChannel::BrightnessUP()      
		{
			// targetChnlBright_percent=(targetChnlBright_percent+1)>MaxBrightness?MaxBrightness:targetChnlBright_percent+1;
			if (targetChnlBright_percent>=MaxBrightness)
			{targetChnlBright_percent=MaxBrightness;}
			else
			{targetChnlBright_percent +=1;}
			// Serial.print(F("targetChnlBright_percent="));
			// Serial.println(targetChnlBright_percent);

		}

    // уменьшаем яркость на 1%
	void LedChannel::BrightnessDOWN()
	    {
			targetChnlBright_percent=targetChnlBright_percent==0? 0:targetChnlBright_percent-1;
		}   



	// функция рассчёта к-та яркости для канала без учёта рассвета/заката ()
	void LedChannel::update_PWM_Level_1 ()
	{
		uint32_t tmpPWM_Level=0;
		tmpPWM_Level = 1*MaxBrightnessABS;
		tmpPWM_Level=tmpPWM_Level*targetChnlBright_percent;
		// tmpPWM_Level=tmpPWM_Level*coeffBright;
		// tmpPWM_Level=tmpPWM_Level/MaxBrightness;

		PWM_channel_level=tmpPWM_Level/MaxBrightness;

		// Serial.print (F("PWM_channel_level=")); Serial.println(PWM_channel_level, DEC);
		// PWM_channel_level=tmpPWM_Level/100;
	}


	// функция рассчёта к-та яркости для канала с учётом рассвета/заката
	void LedChannel::update_PWM_Level (uint8_t &CurrHour, uint8_t &CurrMinute)
	{
		uint16_t currTime;                            
		const uint16_t minInHour = 60 ;             // минут в часе 
		uint8_t coeffBright;                        // расчётный к-т яркости (в %%)

		currTime = (minInHour*CurrHour+CurrMinute); // текушее время = текщий час*60+текущие минуты

		for(int i=0; i<2; i++)                      // у нас два таймера для канала надо обработать оба
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
						coeffBright = 100*timeIntervalFromStartTimer/LedTimerParam[i].dimmingOnDuration;
					}

					// для плавного увеличения яркости				
					else if (currTime >= (LedTimerParam[i].hourOff*minInHour+LedTimerParam[i].minuteOff - LedTimerParam[i].dimmingOffDuration))
					{
						coeffBright = 100*(LedTimerParam[i].hourOff*minInHour+LedTimerParam[i].minuteOff-currTime)/LedTimerParam[i].dimmingOffDuration;
					}
					
					// если не надо снижать или повышать
					else 
					{
						coeffBright=100;						
					}        
		
				{	
					uint32_t tmpPWM_Level=0;
					tmpPWM_Level = 1*MaxBrightnessABS;
					tmpPWM_Level=tmpPWM_Level*targetChnlBright_percent;
					tmpPWM_Level=tmpPWM_Level*coeffBright;
					tmpPWM_Level=tmpPWM_Level/MaxBrightness;
					PWM_channel_level=static_cast<uint16_t>(tmpPWM_Level/100);
				}
				return;
			}   
			else 
			{
				PWM_channel_level = 0;
			}


			
			
			// Serial.print (F("Channel=")); Serial.print (channelName);
			// Serial.print (F("    Timer №_")); Serial.print (i);
			//Serial.print (F(",  PWM_channel_level=")); Serial.println (PWM_channel_level);
			//Serial.print (F("PWM_channel_level="));// Serial.println (PWM_channel_level);
			//  memoryReport("BeforeSerialPrint");
			//  Serial.println ("PWM_channel_level=");// Serial.println (PWM_channel_level);
			// memoryReport("After");
			// 	delay(500);
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