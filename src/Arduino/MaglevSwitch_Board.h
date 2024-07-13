#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "typedef_common.h"
#include "typedef_v0.h"
#include "typedef_v1.h"
#include "Threshold_Data.h"

#ifndef MAGLEV_SWITCH_BOARD
#define MAGLEV_SWITCH_BOARD

const int MLSW_LOWER_LIMIT  = 1070;
const int MLSW_UPPER_LIMIT  = 3170;
const int MLSW_RANGE        = MLSW_UPPER_LIMIT - MLSW_LOWER_LIMIT;

struct EncoderConditions{
  bool A;
  bool B;
  int pulseCount;
  double position;
};

class MaglevSwitchBoard{
public:
    MaglevSwitchBoard();
    MaglevSwitchBoard(uint16_t version, char* swCodesLayer1, char* swCodesLayer2, 
      ThresholdData* actuationDepth, ThresholdData* releaseDepth, uint8_t* outCodes);
    bool updateState(void);
    uint16_t currentMaglevValue(uint8_t);
    uint8_t outCodesCnt(void);
    bool encoderPressed(void);

    bool switchGpioInit(void);
    bool rotaryEncoderInit(void);
    bool i2cInit(void);
    void saveSettings(void);
    void setThreshold(uint16_t* actuationDepth, uint16_t* releaseDepth);

private:
    const uint8_t SW_NUM; 
    const uint8_t DIRECT_SW_NUM;
    const uint8_t MATRIX_OUT_NUM;
    const uint8_t MATRIX_IN_NUM;

    const uint8_t* SW_GPIO; 
    const uint8_t* MATRIX_OUT_GPIO;
    const uint8_t* MATRIX_IN_GPIO;
    const uint32_t* STATUS_BITS;
    const uint32_t* STATUS_BITS_SW;

    char* swCodes_;
    char swCodesLayer1_[SW_NUM_101];
    char swCodesLayer2_[SW_NUM_101];

    uint32_t switchStatusBits_;
    uint32_t switchStatusBitsPrev_;

    uint8_t* outCodes_;
    uint8_t outCodesCnt_;
    bool encoderSwPressed_;
    
    int8_t fnSw_;

    ThresholdData* actuationDepth_;
    ThresholdData* releaseDepth_;
    uint16_t currentDepth_[4];
    uint16_t prevDepth_[4];
    bool rangePrev_[4];
    bool isGoingDownPrev_[4];
    bool pressedPrev_[4];

    bool (MaglevSwitchBoard::*updateFunc)(void);

    void updateEncoderSwitch(void);
    void updateAnalogueSwitch(void);
    void updateDigitalSwitch(void);
    void updateMatrix(void);

    void makeSendCodes(void);
    void switchLayer(void);
    
    uint8_t countSetBits(uint32_t n);
    void getActiveCodes(uint32_t bits, uint8_t distNum);

    bool isPressed(uint8_t idx);
    bool isNegative(uint16_t minuend,uint16_t subtrahend);
    bool getDirection(uint16_t prev, uint16_t current);
    uint8_t getTurning(bool direction, bool directionPrev, uint8_t modePrev);
    bool getStaying(uint16_t prev, uint16_t current, uint8_t range, uint8_t rangePrev);
    uint8_t getRange(uint16_t current, uint16_t release_, uint16_t actuation);

    void loadSettings(void);
};
#endif
