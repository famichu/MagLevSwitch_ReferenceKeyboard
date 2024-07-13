#include "MaglevSwitch_Board.h"
#include <cstring>

MaglevSwitchBoard::MaglevSwitchBoard()
  : SW_NUM        (SW_NUM_DEFAULT),
  DIRECT_SW_NUM   (DIRECT_SW_NUM_DEFAULT),
  MATRIX_OUT_NUM  (MATRIX_OUT_NUM_DEFAULT),
  MATRIX_IN_NUM   (MATRIX_IN_NUM_DEFAULT),
  SW_GPIO         (nullptr),
  MATRIX_OUT_GPIO (nullptr),
  MATRIX_IN_GPIO  (nullptr),
  STATUS_BITS     (nullptr),
  STATUS_BITS_SW    (nullptr)
  {};

MaglevSwitchBoard::MaglevSwitchBoard(uint16_t version, char* swCodesLayer1, char* swCodesLayer2, 
  ThresholdData* actuationDepth, ThresholdData* releaseDepth, uint8_t* outCodes)
  : SW_NUM        ((version == 100) ? SW_NUM_100          : (version == 101) ? SW_NUM_101           : SW_NUM_DEFAULT),
  DIRECT_SW_NUM   ((version == 100) ? DIRECT_SW_NUM_100   : (version == 101) ? DIRECT_SW_NUM_101    : DIRECT_SW_NUM_DEFAULT),
  MATRIX_OUT_NUM  ((version == 100) ? MATRIX_OUT_NUM_100  : (version == 101) ? MATRIX_OUT_NUM_101   : MATRIX_OUT_NUM_DEFAULT),
  MATRIX_IN_NUM   ((version == 100) ? MATRIX_IN_NUM_100   : (version == 101) ? MATRIX_IN_NUM_101    : MATRIX_IN_NUM_DEFAULT),
  SW_GPIO         ((version == 100) ? SW_GPIO_100         : (version == 101) ? SW_GPIO_101          : nullptr),
  MATRIX_OUT_GPIO ((version == 100) ? MATRIX_OUT_GPIO_100 : (version == 101) ? MATRIX_OUT_GPIO_101  : nullptr),
  MATRIX_IN_GPIO  ((version == 100) ? MATRIX_IN_GPIO_100  : (version == 101) ? MATRIX_IN_GPIO_101   : nullptr),
  STATUS_BITS     ((version == 100) ? STATUS_BITS_100     : (version == 101) ? STATUS_BITS_101      : nullptr), 
  STATUS_BITS_SW  ((version == 100) ? STATUS_BITS_SW_100  : (version == 101) ? STATUS_BITS_SW_101   : nullptr)
  {
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

    fnSw_   = -1;

    switchStatusBits_ = 0;
    switchStatusBitsPrev_ = 0;

    for(uint8_t i = 0; i < SW_NUM; i++){
      swCodesLayer1_[i] = swCodesLayer1[i];
      swCodesLayer2_[i] = swCodesLayer2[i];
      if(swCodesLayer1[i] == FN){
        fnSw_ = i;
      }
    }
    
    swCodes_      = swCodesLayer1_;
}

bool MaglevSwitchBoard::switchGpioInit(void){
    for(int i = 0; i < DIRECT_SW_NUM; i++){
      gpio_init(SW_GPIO[i]);
      gpio_pull_up(SW_GPIO[i]);
      gpio_set_dir(SW_GPIO[i], GPIO_IN);
    }
    
    for(int i = 0; i < MATRIX_IN_NUM; i++){
      gpio_init(MATRIX_IN_GPIO[i]);
      gpio_pull_up(MATRIX_IN_GPIO[i]);
      gpio_set_dir(MATRIX_IN_GPIO[i], GPIO_IN);
    }
    for(int i = 0; i < MATRIX_OUT_NUM; i++){
      gpio_init(MATRIX_OUT_GPIO[i]);
      gpio_set_dir(MATRIX_OUT_GPIO[i], GPIO_OUT);
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
  switchStatusBitsPrev_ = switchStatusBits_;
  switchStatusBits_ = 0;

  updateEncoderSwitch();
  updateAnalogueSwitch();
  updateDigitalSwitch();
  updateMatrix(); // it takes a 75us * number of rows

  makeSendCodes();

  if(outCodesCnt_ > 0){
    return true;
  }
  return false;
}

void MaglevSwitchBoard::updateEncoderSwitch(){
  if(!gpio_get(ENCODER_SW)){
    encoderSwPressed_ = true;
  }
  else{
    encoderSwPressed_ = false;
  }
}

void MaglevSwitchBoard::updateAnalogueSwitch(){
  for(int i = 0; i < MLSW_NUM; i++){
    prevDepth_[i] = currentDepth_[i];
    adc_select_input(ADC_NUM[i]);
    currentDepth_[i] = adc_read();

    if(isPressed(i)){
      switchStatusBits_ |= STATUS_BITS_MLSW[i];
    }
  }
}

void MaglevSwitchBoard::updateDigitalSwitch(){
  for(int i = 0; i < DIRECT_SW_NUM; i++){
    if(!gpio_get(SW_GPIO[i])){
      switchStatusBits_ |= STATUS_BITS_SW[i];
    }
  }
}

void MaglevSwitchBoard::updateMatrix(){
  for(uint8_t i = 0; i < MATRIX_OUT_NUM; i++){
    gpio_put(MATRIX_OUT_GPIO[i], 0);
    absolute_time_t switching_delay_time = make_timeout_time_us(75);
    sleep_until(switching_delay_time);

    for(uint8_t j = 0; j < MATRIX_IN_NUM; j++){
      if(!gpio_get(MATRIX_IN_GPIO[j])){
        switchStatusBits_ |= STATUS_BIT_MATRIX[i][j];
      }
    }
    gpio_put(MATRIX_OUT_GPIO[i], 1);
  }
}

void MaglevSwitchBoard::makeSendCodes(){
  switchLayer();

  for(int i = 0; i < 6; i++){
    outCodes_[i] = 0;
  }
  outCodesCnt_ = 0;
  
  uint32_t longPressedSwStatusBits = (switchStatusBits_ & switchStatusBitsPrev_);
  uint8_t longPressedCnt = countSetBits(longPressedSwStatusBits);
  
  /*if(longPressedCnt >= 6){
    getActiveCodes(longPressedSwStatusBits, 6);
  }
  else{
    getActiveCodes(longPressedSwStatusBits, longPressedCnt);
    switchStatusBits_ = (switchStatusBits_ & ~longPressedSwStatusBits);
    */
    getActiveCodes(switchStatusBits_, 6);
  //}
}

void MaglevSwitchBoard::switchLayer(){
  if(fnSw_ != -1){
    if((switchStatusBits_ & STATUS_BITS[fnSw_]) != 0){
      swCodes_ = swCodesLayer2_;
    }
    else{
      swCodes_ = swCodesLayer1_;
    }
  }
}

uint8_t MaglevSwitchBoard::countSetBits(uint32_t n) {
    uint8_t cnt = 0;
    while (n) {
        n &= (n - 1);
        cnt++;
    }
    return cnt;
}

void MaglevSwitchBoard::getActiveCodes(uint32_t bits, uint8_t distNum){
    for(uint8_t i = 0; i < SW_NUM; i++){
      if(i != fnSw_){
        if(outCodesCnt_ == distNum){
          break;
        }
        if(bits & STATUS_BITS[i]){
          outCodes_[outCodesCnt_] = swCodes_[i];
          outCodesCnt_++;
        }
      }
    }
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
  uint8_t range = getRange(currentDepth_[idx], 
    releaseDepth_[idx].getAbsoluted(), 
    actuationDepth_[idx].getAbsoluted());

  bool isGoingDown = getDirection(prevDepth_[idx], currentDepth_[idx]);
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
