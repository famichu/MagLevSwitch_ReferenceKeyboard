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
    
    union{
      uint16_t uint16;
      uint8_t bytes[2];
    } lowerLimit;
    
    union{
      uint16_t uint16;
      uint8_t bytes[2];
    } range;
    
    float normalized;

    float normalizeValue(uint16_t value);
    uint16_t absoluteValue(float value);

  public:
    ThresholdData();
    ThresholdData(uint16_t value1, uint16_t value2, uint16_t value3);
    ThresholdData(float value, uint16_t value2, uint16_t value3);

    void setNormalize(float value);
    void setAbsolute(uint16_t value);
    void setLowerLimit(uint16_t lowerLimit_);
    void setRange(uint16_t upperLimit_);
    void setAbsoluteBytes(uint8_t value0, uint8_t value1);
    void setLowerLimitBytes(uint8_t lowerLimit0, uint8_t lowerLimit1);
    void setRangeBytes(uint8_t range0, uint8_t range1);

    float getNormalized();
    uint16_t getAbsoluted();
    uint16_t getLowerLimit();
    uint16_t getRange();
    uint8_t* getAbsolutedBytes();
    uint8_t* getLowerLimitBytes();
    uint8_t* getRangeBytes();

    ThresholdData& operator=(const ThresholdData& other);
};

#endif
