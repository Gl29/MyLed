#ifndef LEDCHANNEL_H
#define LEDCHANNEL_H

#include "Arduino.h"
#include <string.h>

class LedChannel {
    private:
        const uint8_t   MaxBrightness       =100;   // 100%
        const uint16_t  MaxBrightnessABS    =4096;  // абсолютное значение яркости канала для TLC5940
       
        struct LedTimer                             // в каждом таймере храним два набора данных 1)параметры включения  2)параметры выключения
        {
            uint8_t hourOn              =0;
            uint8_t minuteOn            =0;
            uint8_t dimmingOnDuration   =0;

            uint8_t hourOff             =0;
            uint8_t minuteOff           =0;
            uint8_t dimmingOffDuration  =0;

        };

    public:
        char *channelName;                        // текстовое имя LED канала 
        uint8_t targetChannelBrightness;          // значение в процентах максимально заданной яркости канала 
                                                  // микросхема  TLC5940NT - 16-канальный ШИМ драйвер 
                                                  // с 12-битным регулированием скважности (0-4096) и 6-битным (0-63) регулированием тока.
        
        uint32_t PWM_channel_level;               // значение яркости канала (абсолютное)  в текущий момент времени (с учётом таймеров и планого изменения яркости)
        uint8_t *ptr_channelBrightness;           // указатель на переменную channelBrightness
       // enum t_BlinkMode {BlinkOff=1, BlinkRowOn, BlinkRowOFF, BlinkParamON, BlinkParamOFF} blink; 
        LedTimer LedTimerParam[2];                // инициируем 2 таймера для каждого Лед канала



    /**	Конструктор класса **/	
    LedChannel ( char *, uint8_t );   

    void update_PWM_Level (uint8_t, uint8_t);
    void SetTimer(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);       // прописываем параметры таймера
    
  //  uint8_t *GetlBrightness()     {return &channelBrightness;}                                       // возвращаем яркость в %%
    void    SetBrightness(uint8_t v)     {targetChannelBrightness=v>MaxBrightness?MaxBrightness:v;}      // задаём яркость в %%
    void    BrightnessUP()      {targetChannelBrightness=targetChannelBrightness+1>MaxBrightness?MaxBrightness:targetChannelBrightness+1;}; // увеличиваем яркость на 1%
    void    BrightnessDOWN()    {targetChannelBrightness=targetChannelBrightness==0? 0:targetChannelBrightness-1;};   // уменьшаем яркость на 1%

};

#endif