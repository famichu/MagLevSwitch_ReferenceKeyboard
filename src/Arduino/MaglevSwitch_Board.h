#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"

#ifndef MAGLEV_SWITCH_BOARD
#define MAGLEV_SWITCH_BOARD

const char FN = 0x00;

const uint8_t SW_NUM    = 21;
const uint8_t MLSW_NUM  = 4;

const int MLSW_LOWER_LIMIT  = 1070;
const int MLSW_UPPER_LIMIT  = 3170;
const int MLSW_RANGE        = MLSW_UPPER_LIMIT - MLSW_LOWER_LIMIT;

const uint8_t SW_GPIO[] = {
  20, 19, 18, 17, 16, 15, 
  21, 22,     24, 23, 
  5,          25, 
  6,  4,  1,  0, 
  7,  8,  9,  10, 11
};

const uint8_t MLSW_GPIO[] = {
  26, 
  29, 28, 27
};

const uint8_t adc_num[] = {
  0, 
  3, 2, 1
};

const uint8_t ENCODER_A   = 12;
const uint8_t ENCODER_B   = 14;
const uint8_t ENCODER_SW  = 13;

struct EncoderConditions{
  bool A;
  bool B;
  int pulseCount;
  double position;
};

class MaglevSwitchBoard{
public:
    MaglevSwitchBoard();
    MaglevSwitchBoard(char* swCodesLayer1, char* swCodesLayer2, 
      char* mlswCodesLayer1, char* mlswCodesLayer2,  
      float* actuationDepth, float* releaseDepth, uint8_t* outCodes);
    bool updateState(void);
    uint16_t currentMaglevValue(uint8_t);
    uint8_t outCodesCnt(void);
    bool encoderPressed(void);

    bool switchGpioInit(void);
    bool rotaryEncoderInit(void);
    bool i2cInit(void);

    void setThresholdRate(float* actuationDepth, float* releaseDepth);
    void setThresholdAbsolute(uint16_t* actuationDepth, uint16_t releaseDepth);

private:
    char swPins_[SW_NUM];
    char* swCodes_;
    char* swCodesLayer1_;
    char* swCodesLayer2_;
    char* mlswCodes_;
    char* mlswCodesLayer1_;
    char* mlswCodesLayer2_;

    uint8_t* outCodes_;
    uint8_t outCodesCnt_;
    bool encoderSwPressed_;
    
    int fnSw_;
    int swNum_;

    uint16_t actuationDepth_[4];
    uint16_t releaseDepth_[4];
    uint16_t currentDepth_[4];
    
    bool (MaglevSwitchBoard::*updateFunc)(void);

    bool updateWithLayers_(void);
    bool update_(void);
};
#endif
