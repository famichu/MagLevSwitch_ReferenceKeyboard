#include <stdio.h>

#ifndef THRESHOLDDATA_H
#define THRESHOLDDATA_H

class ThresholdData
{
  private:
    union{
      uint16_t uint16;
      uint8_t bytes[2];
    } absoluted;
    
    float normalized;

    uint16_t lowerLimit;
    uint16_t range;
    
    float normalizeValue(uint16_t value);
    uint16_t absoluteValue(float value);

  public:
    ThresholdData();
    ThresholdData(uint16_t value1, uint16_t value2, uint16_t value3);
    ThresholdData(float value, uint16_t value2, uint16_t value3);

    void setAbsolute(uint16_t value);
    void setNormalize(float value);
    void setAbsoluteBytes(uint8_t value0, uint8_t value1);

    uint16_t getAbsoluted();
    float getNormalized();
    uint8_t* getAbsolutedBytes();

    ThresholdData& operator=(const ThresholdData& other);
};

#endif
