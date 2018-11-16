#ifndef LEDCHANNEL_H
#define LEDCHANNEL_H

#include "Arduino.h"
#include <string.h>

class LedChannel {
    private:
      const uint8_t MaxBrightness       =100; // 100%
    
    public:
        uint8_t channelBrightness;                     // значение яркости канала  микросхема 
                                                        // TLC5940NT - 16-канальный ШИМ драйвер 
                                                        // с 12-битным регулированием скважности (0-4096) и 6-битным (0-63) регулированием тока.
        
        uint8_t *ptr_channelBrightness;                 // указатель на переменную channelBrightness
        enum t_BlinkMode {BlinkOff=1, BlinkRowOn, BlinkRowOFF, BlinkParamON, BlinkParamOFF} blink; 
        char      *channelName;                         // текстовое имя LED канала     
   


    /**	Конструктор класса **/	
    LedChannel ( char *_Name, uint8_t _cBR, t_BlinkMode _blk=BlinkOff) //, uint8_t _LCDScreenRowNumb = 0)   
    {
        channelName         = _Name;
        channelBrightness   =_cBR>MaxBrightness?MaxBrightness:_cBR;
        blink               =_blk;
    //    LcdRowNumber        = _LCDScreenRowNumb;
        ptr_channelBrightness = &channelBrightness;

    }    
    


  //  uint8_t *GetlBrightness()     {return &channelBrightness;}                                       // возвращаем яркость в %%
    void    SetBrightness(uint8_t v)     {channelBrightness=v>MaxBrightness?MaxBrightness:v;}      // задаём яркость в %%
    void    BrightnessUP()      {channelBrightness=channelBrightness+1>MaxBrightness?MaxBrightness:channelBrightness+1;}; // увеличиваем яркость на 1%
    void    BrightnessDOWN()    {channelBrightness=channelBrightness==0? 0:channelBrightness-1;};   // уменьшаем яркость на 1%

};

#endif