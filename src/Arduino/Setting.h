#include "Threshold_Data.h"

#ifndef SETTING_H
#define SETTING_H

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Adafruit_TinyUSB.h"
#include "hid/Adafruit_USBD_HID.h"
    
 struct Config{
     uint16_t HardwareVersion;
     uint16_t SoftwareVersion;
     ThresholdData ActuationThresholds[4];
     ThresholdData ReleaseThresholds[4];
     char KeymapL1[28];
     char KeymapL2[28];
 };

class Setting{
    public: 
        Setting();
        Setting(uint16_t softwareVersion);
        bool Initialize(void);
        bool Initialize(uint16_t hardwareVersion);
        uint8_t Calibration(Adafruit_SSD1306& display, uint8_t page);
        void SetThresholds(ThresholdData* actuationThresholds, ThresholdData* releaseThresholds);
        bool SetKeymap(uint8_t* map, uint8_t keynum);
        void Save(void);
        Config Load();
        Config getConfig() const;
    private: 
        Config config;
        bool load_succeeded = false;
};

#endif
