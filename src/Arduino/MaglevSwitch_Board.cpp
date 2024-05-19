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
      reActivationDepth_[i] = releaseDepth_[i].getAbsoluted();
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

// get whether the switch moving direction is turned. true: turned
bool MaglevSwitchBoard::getTurning(bool direction, bool directionPrev, int statePrev){
  if(statePrev >= 2){
    if(direction == false){
      if(directionPrev == true){
        return true;
      }
    }
  }
  
  return false;
}

// get the state of the switch
uint8_t MaglevSwitchBoard::getState(uint16_t current, uint16_t release_, uint16_t actuation, uint16_t disable){
  bool isBottom = isNegative(current, release_);    // lower than the depth at which the switch was released
  bool isActuated = isNegative(current, actuation); // lower than the actuation depth of the switch
  bool isDisabled = isNegative(current, disable);   // lower than the ignore depth after the switch was released

  if(isBottom){
    return 3; // the switch is pressed almost fully
  }
  else if(isDisabled){ // the switch has just been released, and current state is ignored
    return 1; 
  }
  else if(isActuated){
    return 2; // the switch is pressed
  }
  else{
    return 0; // the switch is not pressed
  }
}

// get whether the switch is pressed based on the state of the switch
bool MaglevSwitchBoard::isPressed(uint8_t idx){
  bool pressed = false;
  bool isGoingDown = getDirection(prevDepth_[idx], currentDepth_[idx]);
  bool isTurning = getTurning(isGoingDown, directionPrev_[idx], statePrev_[idx]);
  uint8_t state = getState(currentDepth_[idx], 
    releaseDepth_[idx].getAbsoluted(), 
    actuationDepth_[idx].getAbsoluted(), 
    reActivationDepth_[idx]);

  switch(state){
    case 2:
      if(isTurning == true){
        reActivationDepth_[idx] = currentDepth_[idx] + (MLSW_RANGE * 0.2);
      }
    case 3:
      pressed = true;
      break;
  }
  
  statePrev_[idx] = state;
  directionPrev_[idx] = isGoingDown;

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
