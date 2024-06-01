#include "MaglevSwitch_Board.h"
#include <cstring>

MaglevSwitchBoard::MaglevSwitchBoard(){};

MaglevSwitchBoard::MaglevSwitchBoard(char* swCodesLayer1, char* swCodesLayer2, 
  char* mlswCodesLayer1, char* mlswCodesLayer2,  
  ThresholdData* actuationDepth, ThresholdData* releaseDepth, uint8_t* outCodes){

    memcpy(&swCodesLayer1_, &swCodesLayer1, 21);
    memcpy(&swCodesLayer2_, &swCodesLayer2, 21);
    memcpy(&mlswCodesLayer1_, &mlswCodesLayer1, 4);
    memcpy(&mlswCodesLayer2_, &mlswCodesLayer2, 4);
    
    swCodes_  = swCodesLayer1_;

    outCodes_ = outCodes;

    switchGpioInit(); 

    actuationDepth_ = actuationDepth;
    releaseDepth_ = releaseDepth;

    for(int i = 0; i <MLSW_NUM; i++){
      currentDepth_[i]      = 0;
      pressedPrev_[i]       = false;
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
    
    swCodes_    = swCodesLayer1_;
    mlswCodes_  = mlswCodesLayer1_;
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
    prevDepth_[i] = currentDepth_[i];
    adc_select_input(adc_num[i]);
    currentDepth_[i] = adc_read();

    if(isPressed(i)){
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

// get the current moving direction of the switch. true: is going down.
bool MaglevSwitchBoard::getDirection(uint16_t prev, uint16_t current){
  if(isNegative(current, prev)){
    return true;
  }
  else{
    return false;
  }
}

// get whether the switch moving direction is turned. 0: not turned, 1: turned to down, 2: turned to up 
uint8_t MaglevSwitchBoard::getTurning(bool isGoingDown, bool isGoingDownPrev, uint8_t statePrev){
  if(isGoingDown == false){
    if(isGoingDownPrev == true){
      return 2;
    }
  }
  else if(isGoingDown == true){
    if(isGoingDownPrev == false){
      return 1;
    }
  }
  
  return 0;
}

bool MaglevSwitchBoard::getStaying(uint16_t prev, uint16_t current, uint8_t range, uint8_t rangePrev){
  if(rangePrev != range){
    return false;
  }
  else if((isNegative(current, prev - 3)) || (isNegative(current - 3, prev))){
    return true;
  }
  else{
    return false;
  }
}

// get the state of the switch
uint8_t MaglevSwitchBoard::getRange(uint16_t current, uint16_t release_, uint16_t actuation){
  bool isBottom = isNegative(current, release_);    // lower than the depth at which the switch was released
  bool isActuated = isNegative(current, actuation); // lower than the actuation depth of the switch

  if(isBottom){
    return 2; // lower than the minimum relasing threshold
  }
  else if(isActuated){
    return 1; // between the actuation threshold and the minimum releasing threshold
  }
  else{
    return 0; // higher than the actuation threshold
  }
}

// get whether the switch is pressed based on the state of the switch
bool MaglevSwitchBoard::isPressed(uint8_t idx){
  bool pressed = false;
  bool isGoingDown = getDirection(prevDepth_[idx], currentDepth_[idx]);
  uint8_t range = getRange(currentDepth_[idx], 
    releaseDepth_[idx].getAbsoluted(), 
    actuationDepth_[idx].getAbsoluted());

  bool isStaying = getStaying(prevDepth_[idx], currentDepth_[idx], range, rangePrev_[idx]);
  uint8_t isTurning = 0;
  if(isStaying == false){
    isTurning = getTurning(isGoingDown, isGoingDownPrev_[idx], rangePrev_[idx]);
  }

  switch(range){
    case 1:
      switch(isTurning){
        case 0:
        case 1:
          pressed = true;
          break;
        case 2:
          pressed = false;
          break;
      }
      break;
    case 2:
      pressed = true;
      break;
  }
  
  rangePrev_[idx] = range;
  isGoingDownPrev_[idx] = isGoingDown;
  pressedPrev_[idx] = pressed;

  return pressed;
}

// Calculates whether the difference between the first and second arguments is negative.
bool MaglevSwitchBoard::isNegative(uint16_t minuend, uint16_t subtrahend){
  int16_t diff = minuend - subtrahend;
  int16_t msb = diff >> 15;

  if(msb == 0){
    return false;
  }
  else{
    return true;
  }
}
