#include "MaglevSwitch_Board.h"
#include <cstring>

MaglevSwitchBoard::MaglevSwitchBoard(){};

MaglevSwitchBoard::MaglevSwitchBoard(char* swCodesLayer1, char* swCodesLayer2, 
  char* mlswCodesLayer1, char* mlswCodesLayer2,  
  float* actuationDepth, float* releaseDepth, uint8_t* outCodes){

    memcpy(&swCodesLayer1_, &swCodesLayer1, 21);
    memcpy(&swCodesLayer2_, &swCodesLayer2, 21);
    memcpy(&mlswCodesLayer1_, &mlswCodesLayer1, 4);
    memcpy(&mlswCodesLayer2_, &mlswCodesLayer2, 4);
    
    swCodes_  = swCodesLayer1_;

    outCodes_ = outCodes;

    switchGpioInit(); 
    
    for(int i = 0; i <MLSW_NUM; i++){
      actuationDepth_[i] = MLSW_LOWER_LIMIT + 
        (uint16_t)((float)MLSW_RANGE * actuationDepth[i]);
      releaseDepth_[i]   = MLSW_LOWER_LIMIT + 
        (uint16_t)((float)MLSW_RANGE * releaseDepth[i]);

      currentDepth_[i]   = 0;
    }

    rotaryEncoderInit();
    i2cInit();

    fnSw_   = 0;
    swNum_  = 0;

    updateFunc = &MaglevSwitchBoard::update_;

    for(int i = 0; i < SW_NUM; i++){
      if(swCodes_[i] == FN){ // Make enable Fn key scan if Fn key was used
        fnSw_ = SW_GPIO[i];
        updateFunc = &MaglevSwitchBoard::updateWithLayers_;
      }
      else{
        swCodesLayer1_[swNum_]  = swCodesLayer1[i];
        swCodesLayer2_[swNum_]  = swCodesLayer2[i];
        swPins_[swNum_] = SW_GPIO[i]; // put io number again other than Fn key.
        swNum_++;
      }
    }
}

bool MaglevSwitchBoard::switchGpioInit(void){
    for(int i = 0; i < SW_NUM; i++){
      gpio_init(SW_GPIO[i]);
      gpio_pull_up(SW_GPIO[i]);
      gpio_set_dir(SW_GPIO[i], GPIO_IN);
    }

    adc_init();

    for(int i = 0; i <MLSW_NUM; i++){
      adc_gpio_init(MLSW_GPIO[i]);
    }

    return true;
}

bool MaglevSwitchBoard::rotaryEncoderInit(){
  gpio_init(ENCODER_A);
  gpio_pull_up(ENCODER_A);
  gpio_set_dir(ENCODER_A, GPIO_IN);

  gpio_init(ENCODER_B);
  gpio_pull_up(ENCODER_B);
  gpio_set_dir(ENCODER_B, GPIO_IN);
  
  gpio_init(ENCODER_SW);
  gpio_pull_up(ENCODER_SW);
  gpio_set_dir(ENCODER_SW, GPIO_IN);

  return true;
}

bool MaglevSwitchBoard::i2cInit(){
  gpio_set_function(2, GPIO_FUNC_I2C);
  gpio_set_function(3, GPIO_FUNC_I2C);
  gpio_pull_up(2);
  gpio_pull_up(3);

  return true;
}

bool MaglevSwitchBoard::updateState(void){
  return (this->*updateFunc)();
}

bool MaglevSwitchBoard::updateWithLayers_(void){
  if(!gpio_get(fnSw_)){
    swCodes_    = swCodesLayer2_;
    mlswCodes_  = mlswCodesLayer2_;
  }
  else{
    swCodes_    = swCodesLayer1_;
    mlswCodes_  = mlswCodesLayer1_;
  }

  return update_();
}

bool MaglevSwitchBoard::update_(void){
  for(int i = 0; i < 6; i++){
    outCodes_[i] = 0;
  }
  
  outCodesCnt_ = 0;

  for(int i = 0; i < MLSW_NUM; i++){
    adc_select_input(adc_num[i]);
    currentDepth_[i] = adc_read();

    if(currentDepth_[i] < actuationDepth_[i]){
      outCodes_[outCodesCnt_] = mlswCodes_[i];
      outCodesCnt_++;
    }
  }
  
  for(int i = 0; i < swNum_; i++){
    if(!gpio_get(swPins_[i])){
      outCodes_[outCodesCnt_] = swCodes_[i];
      outCodesCnt_++;

      if(outCodesCnt_ > 5){
        break;
      }
    }
  }

  if(!gpio_get(ENCODER_SW)){
    encoderSwPressed_ = true;
  }
  else{
    encoderSwPressed_ = false;
  }

  if(outCodesCnt_ > 0){
    return true;
  }
  return false;
}

uint16_t MaglevSwitchBoard::currentMaglevValue(uint8_t idx){
  if(idx > MLSW_NUM){
    return 0;
  }
  return currentDepth_[idx];
}

uint8_t MaglevSwitchBoard::outCodesCnt(){
  return outCodesCnt_;
}

bool MaglevSwitchBoard::encoderPressed(){
  return encoderSwPressed_;
}

// set thresholds with float value(0.0 - 1.0)
void MaglevSwitchBoard::setThresholdRate(float* actuationDepth, float* releaseDepth){
  for(int i = 0; i <MLSW_NUM; i++){
    actuationDepth_[i] = MLSW_LOWER_LIMIT + 
      (uint16_t)((float)MLSW_RANGE * actuationDepth[i]);
    releaseDepth_[i]   = MLSW_LOWER_LIMIT + 
      (uint16_t)((float)MLSW_RANGE * releaseDepth[i]);
  }
}

// set thresholds with integer value as absolute value
void MaglevSwitchBoard::setThresholdAbsolute(uint16_t* actuationDepth, uint16_t releaseDepth){
  memcpy(&actuationDepth_, &actuationDepth, 4);
  memcpy(&releaseDepth_, &releaseDepth, 4);
}
