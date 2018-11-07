#ifndef LEDCHANNEL_H
#define LEDCHANNEL_H

#include "Arduino.h"
#include <string.h>

class LedChannel {
    private:
      const uint8_t MaxBrightness       =100; // 100%
            uint8_t channelBrightness   =MaxBrightness/3;   // значение яркости канала  микросхема 
                                                            // TLC5940NT - 16-канальный ШИМ драйвер 
                                                            // с 12-битным регулированием скважности (0-4096) и 6-битным (0-63) регулированием тока.

    public:
        enum t_BlinkMode {BlinkOff=1, BlinkRowOn, BlinkRowOFF, BlinkParamON, BlinkParamOFF} blink; 
       // String      channelName;              // текстовое имя LED канала 
         char      *channelName;              // текстовое имя LED канала     
        byte        channelNumber;              // номер  LED канала 
        // uint8_t     blink;                      // хранит признак что надо мигать параметром и текущий статус мигания:uint8_t
        //                                         // 0 - режим мигания выключен
        //                                         // 1 - режим мигания включён, параметр на экран НЕ выводим
        //                                         // 2 - режим мигания включён, параметр на экран    выводим
        uint8_t     LcdRowNumber;               // номер экрана и строк на которых выводятся параметры данного канала

    /**	Конструктор класса **/	
 //   LedChannel (String _Name,byte _channelNumber, uint16_t _cBR, bool _blk = false, uint8_t _LCDScreenNumb = 0)
    LedChannel ( char *_Name, //byte _channelNumber, 
                uint8_t _cBR, t_BlinkMode _blk=BlinkOff, uint8_t _LCDScreenRowNumb = 0)   
    {
       // *channelName=_Name;
       //strcpy(channelName,_Name);
        channelName         = _Name;
      //  channelNumber       =_channelNumber;
        channelBrightness   =_cBR>MaxBrightness?MaxBrightness:_cBR;
        blink               =_blk;
        LcdRowNumber        = _LCDScreenRowNumb;

    }    
    


    uint8_t *GetlBrightness()     {return &channelBrightness;}                                       // возвращаем яркость в %%
    void    SetBrightness(uint8_t v)     {channelBrightness=v>MaxBrightness?MaxBrightness:v;}      // задаём яркость в %%
    void    BrightnessUP()      {channelBrightness=channelBrightness+1>MaxBrightness?MaxBrightness:channelBrightness+1;}; // увеличиваем яркость на 1%
    void    BrightnessDOWN()    {channelBrightness=channelBrightness==0? 0:channelBrightness-1;};   // уменьшаем яркость на 1%

};

#endif