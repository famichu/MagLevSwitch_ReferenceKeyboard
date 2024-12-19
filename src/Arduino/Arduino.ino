#include "MaglevSwitch_Board.h"
#include <cstdint>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Adafruit_TinyUSB.h"
#include "hid/Adafruit_USBD_HID.h"
#include "BarGraph.h"
#include "Setting.h"
#include "Demo.h"

// #define DISPLAY_DEBUG // Showing the debugging screen
#define DEMO_MODE

// Display
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

i2c_inst_t *i2c = i2c1;
TwoWire myWire(i2c, 2, 3);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &myWire, -1);

#define NUM_BARS 4
BarGraph graph(SCREEN_WIDTH, SCREEN_HEIGHT, NUM_BARS);

// USB HID
Adafruit_USBD_HID usb_hid;

uint8_t const desc_hid_report[] = {
  TUD_HID_REPORT_DESC_KEYBOARD()
};


// Rotary encoder
volatile uint64_t lastInterruptTimeUs = 0; 
static uint64_t debounceDelayUs = 1500; // Debounce delay time(us)
volatile EncoderConditions encoderConditions;
void encoderInterrupt(void); // Prototype


// Semaphoere
static semaphore_t semaphoere;

static uint8_t SwVersion = 101;
uint16_t HwVersion = 0;

// MagLevSwitch Thresholds
// prototype
float calcNormalizedValue(uint16_t);
float calcAbsoluteValue(uint16_t);

// Output char codes array
uint8_t outCodes[6] = {0};

// Current oled state
int8_t G_OLEDSTATE = -1;


MaglevSwitchBoard* board = nullptr;
Setting setting = Setting(101);
Config config;

uint8_t G_LOADED = 0;


void setup() {
  i2c_init(i2c, 400 * 1000);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  
  #ifndef DEMO_MODE
  hid_init();
  #endif
  #ifdef DEMO_MODE
  eraseSettings();
  #endif

  resetWizard();

  if(setting.Initialize() == false) {
    calibrationWizard();
  }
  config = setting.getConfig();

  board = new MaglevSwitchBoard(config.HardwareVersion, config.KeymapL1, config.KeymapL2, 
      config.ActuationThresholds, config.ReleaseThresholds, outCodes);

  showLogo();

  attachInterrupt(digitalPinToInterrupt(ENCODER_A), encoderInterrupt, RISING);

  sem_init(&semaphoere, 1, 1);
  multicore_launch_core1(pollingLoop);
}

void loop() {
  const uint32_t intervalMs = 50;
  uint32_t prevMs           = 0;
  uint32_t currentMs        = 0;

  uint8_t cnt         = 0;
  uint8_t codes[6]    = {0};

  uint16_t values[4]  = {0};
  float normalizedValue[4] = {0.0};

  bool pressed        = false;
  bool pressedPrev    = false;

  static uint8_t oledstatePrev = 0;

  while(1){
    pressedPrev   = pressed;
    currentMs     = to_ms_since_boot(get_absolute_time());

    sem_acquire_blocking(&semaphoere);
    
    cnt = board->outCodesCnt();
    memcpy(&codes, &outCodes, 6);
    for(int i = 0; i < 4; i++){
      values[i] = board->currentMaglevValue(i);
    }
 
    if(currentMs - prevMs >= intervalMs){
      prevMs  = currentMs;
      pressed = board->encoderPressed();
    }

    sem_release(&semaphoere);

    #if !defined(DEMO_MODE)
    hid_task(codes, cnt);
    #endif
    
    #ifdef DISPLAY_DEBUG
    for(uint8_t i = 0; i < cnt; i++){
      codes[i] = codes[i] + 61;
    }
    #endif
    
   for(int i = 0; i < 4; i++){
     normalizedValue[i] = calcNormalizedValue(values[i]);
   }

   if(pressed & !pressedPrev){
     G_OLEDSTATE++;

     if(G_OLEDSTATE == 9){
         sem_acquire_blocking(&semaphoere);
         setting.Save(config);
         sem_release(&semaphoere);
         break;
     }

     if(G_OLEDSTATE > 8){
       G_OLEDSTATE = -1;
     }
     oledstatePrev = G_OLEDSTATE;
   }

    updateOled(codes, cnt, normalizedValue, encoderConditions.position, G_OLEDSTATE);
  }
}

void pollingLoop(void){
  const uint64_t intervalUs = 1000;
  static uint64_t startUs   = 0;
  
  while(1){
    if(to_us_since_boot(get_absolute_time()) - startUs < intervalUs){
      continue;
    }
    startUs += intervalUs;
    
    sem_acquire_blocking(&semaphoere);
    board->updateState();
    sem_release(&semaphoere);
  }
}

void updateOled(uint8_t* codes, uint8_t cnt, float values[4], double position, int8_t oledState){
  static int8_t prevOledState = -1;

  if(oledState == -1){
    if(prevOledState != -1){
      showLogo();
    }
  }
  else{
    display.clearDisplay();

    #ifdef DISPLAY_DEBUG
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 10);
    display.println("MagLev Switch MX");
    display.setCursor(100, 10);
    display.println(config.HardwareVersion);

   for(int i = 0; i < 4; i++){
     display.setCursor(0, (20 + 10 * i));
     display.println(calcAbsoluteValue(values[i]));
   }

   for(int i = 0; i < 4; i++){
     display.setCursor(30, (20 + 10 * i));
     display.println(config.ActuationThresholds[i].getAbsoluted());
   }
   
   for(int i = 0; i < 4; i++){
     display.setCursor(60, (20 + 10 * i));
     display.println(config.ReleaseThresholds[i].getAbsoluted());
   }
    
    display.setCursor(100, 20);
    display.println(position);
    
    display.setCursor(100, 30);
    display.println(oledState);

    display.setCursor(100, 40);
    display.println(cnt);
    
    display.setCursor(95, 50);
    char *charCodes = (char *)codes;
    display.println(charCodes);
    
    #else
    graph.cursor = oledState;
    for (int i = 0; i < NUM_BARS; ++i) {;
      graph.setBarValue(i,  values[i]);
      #if !defined(DEMO_MODE)
      graph.setBarThresholds(
        i, 
        config.ReleaseThresholds[i].getNormalized(),
        config.ActuationThresholds[i].getNormalized());
      #endif
    }
    graph.draw(display);
    
    #ifdef DEMO_MODE
    char *charCodes = (char *)codes;
    display.setTextSize(1);
    display.setTextColor(WHITE);
    
    for(uint8_t i = 0; i < cnt; i++){
      display.setCursor(0, 10 * i);
      uint8_t keyNum = getSwitchNum(charCodes[i]);
      display.println(keyNum);
    } 
    #endif
    #endif
    display.display();
  }

  prevOledState = oledState;
}

void showLogo(void){
  display.clearDisplay();
  display.drawXBitmap(0, 0, LOGO, 128, 64, 1);
  display.display();
}

void encoderInterrupt(void){
  uint64_t currentTimeUs = to_us_since_boot(get_absolute_time());

  if(currentTimeUs - lastInterruptTimeUs >= debounceDelayUs){
    lastInterruptTimeUs = currentTimeUs;
    if(digitalRead(ENCODER_B) == HIGH){
        encoderConditions.pulseCount--;
        increaseThreshold(G_OLEDSTATE, false);
    }
    else{
        encoderConditions.pulseCount++;
        increaseThreshold(G_OLEDSTATE, true);
    }
    
    if(encoderConditions.pulseCount >= 24){
      encoderConditions.pulseCount = 0;
    }
    else if(encoderConditions.pulseCount < 0){
      encoderConditions.pulseCount = 23;
    }
    encoderConditions.position = (double)(encoderConditions.pulseCount) / 23.0;
  }
}

void increaseThreshold(uint8_t state, bool increasing){
  uint8_t index = (state - 1) >> 1;
  uint8_t offset = (state - 1) & 1;
  
  uint16_t incremental = 0;
  if(increasing){
    incremental = MLSW_RANGE / 20;
  }
  else{
    incremental = -MLSW_RANGE / 20;
  }

  if(offset == 0) {
    uint16_t result = config.ActuationThresholds[index].getAbsoluted() + incremental;
    if(result < (MLSW_UPPER_LIMIT - 2) && result >= config.ReleaseThresholds[index].getAbsoluted()){
      config.ActuationThresholds[index].setAbsolute(result);
    }
    else if(result >= (MLSW_UPPER_LIMIT - 2)){
      config.ActuationThresholds[index].setAbsolute(MLSW_UPPER_LIMIT - 3);
    }
    else if(result < config.ReleaseThresholds[index].getAbsoluted()){
      config.ActuationThresholds[index].setAbsolute(config.ReleaseThresholds[index].getAbsoluted());
    }
  }
  else{
    uint16_t result = config.ReleaseThresholds[index].getAbsoluted() + incremental;
    if(result <= config.ActuationThresholds[index].getAbsoluted() && result > (MLSW_LOWER_LIMIT + 50)){
      config.ReleaseThresholds[index].setAbsolute(result);
    }
    else if(result > config.ActuationThresholds[index].getAbsoluted()){
      config.ReleaseThresholds[index].setAbsolute(config.ActuationThresholds[index].getAbsoluted());
    }
    else if(result <= (MLSW_LOWER_LIMIT + 50)){
      config.ReleaseThresholds[index].setAbsolute(MLSW_LOWER_LIMIT + 51);
    }
  }
}

void hid_init(){
  usb_hid.setBootProtocol(HID_ITF_PROTOCOL_KEYBOARD);
  usb_hid.setPollInterval(2);
  usb_hid.setReportDescriptor(desc_hid_report, sizeof(desc_hid_report));
  usb_hid.setStringDescriptor("TinyUSB Keyboard");
  usb_hid.begin();
  while( !TinyUSBDevice.mounted() ) delay(1);
}

void hid_task(uint8_t codes[6], uint8_t cnt){
  static bool keyPressedPreviously = false;
  if ( TinyUSBDevice.suspended() && cnt )
  {
    TinyUSBDevice.remoteWakeup();
  }

  if (!usb_hid.ready()) return;

  if (cnt)
  {
    uint8_t const report_id = 0;
    uint8_t const modifier  = 0;

    keyPressedPreviously = true;
    usb_hid.keyboardReport(report_id, modifier, codes);
  }
  else
  {
    if (keyPressedPreviously)
    {
      keyPressedPreviously = false;
      usb_hid.keyboardRelease(0);
    }
  }
}

float calcNormalizedValue(uint16_t value){
  float result = float(value - MLSW_LOWER_LIMIT) / (float)MLSW_RANGE;
  if(result > 1.00){
    result = 1.00;
  }
  else if(result < 0){
    result = 0.0;
  }
  return result;
}

uint16_t calcAbsoluteValue(float value){
  return uint16_t(value * MLSW_RANGE) + MLSW_LOWER_LIMIT;
}
 
void eraseSettings(){
  const uint32_t FLASH_TARGET_OFFSET = 0x1F0000; // The head address of the final block(block32) of the W25Q16JV: 0x1F0000
  
  uint32_t ints = save_and_disable_interrupts();
  flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
  restore_interrupts(ints);
}

void calibrationWizard(){
  calibrationGpioInit();
  #if !defined(DEMO_MODE)
  config = setting.Initialize(detectHwVersion());
  
  while(gpio_get(13)){
    delay(50);
  }

  clear_gpio(9);
  clear_gpio(23);
  clear_gpio(25);
  clear_gpio(13);
  #else
  config = setting.Initialize(101);
  #endif
  
  setting.Save(config);
}

void calibrationGpioInit(){
  // row 3 of the matrix on v1.01
  gpio_init(9);
  gpio_set_dir(9, GPIO_OUT);
  
  // col 3 of the matrix on v1.01
  gpio_init(23);
  gpio_pull_up(23);
  gpio_set_dir(23, GPIO_IN);
  
  // F key on the both version
  gpio_init(25);
  gpio_pull_up(25);
  gpio_set_dir(25, GPIO_IN);

  // A menu button
  gpio_init(13);
  gpio_pull_up(13);
  gpio_set_dir(13, GPIO_IN);
}

uint16_t detectHwVersion(){
  uint16_t result = 0;
  
  gpio_put(9, 0);
  absolute_time_t switching_delay_time = make_timeout_time_us(75);
  sleep_until(switching_delay_time);

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.println("Calibration wizard");
  display.setCursor(0, 20);
  
  display.println("Please press the key");
  display.setCursor(0, 28);
  display.println("at the far right of");
  display.setCursor(0, 36);
  display.println("the third row.");
  display.display();
  display.setCursor(0, 50);

  while(1){
    if(gpio_get(23) == 0){
      display.println("HW version is 1.01.");
      result = 101;
      break;
    }
    else if(gpio_get(25) == 0){
      display.println("HW version is 1.00.");
      result = 100;
      break;
    }
  }

  display.display();
  return result;
}

void calibrationMagLevRange(){
  adc_init();
  adc_gpio_init(26);
  adc_gpio_init(29);
  adc_gpio_init(28);
  adc_gpio_init(27);
  
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.println("Please stay away from keys and press the menu button.");
  while(gpio_get(13)){
    delay(50);
  }

  
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.println("Please press the A key until the bottom, then release it and press the menu button");
  while(gpio_get(13)){
    delay(50);
  }
}

void clear_gpio(uint pin) {
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_IN);
    gpio_disable_pulls(pin);
}

void resetWizard(){
  // A menu button initialization
  gpio_init(13);
  gpio_pull_up(13);
  gpio_set_dir(13, GPIO_IN);
  
  absolute_time_t switching_delay_time = make_timeout_time_us(75);
  sleep_until(switching_delay_time);

  display.clearDisplay();

  if(gpio_get(13) == 0){
    display.setTextColor(WHITE);
    display.setCursor(0, 10);
    display.println("Press the menu button to reset settings.");
    display.display();
    
    // wait until the button released
    while(gpio_get(13) == 0){
      delay(50);
    }

    // wait until the button pressed
    while(gpio_get(13)){
      delay(50);
    }

    // erase settings
    eraseSettings();
  }
  clear_gpio(13);

  return;
}
