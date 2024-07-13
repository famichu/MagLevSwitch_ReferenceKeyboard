#include "Threshold_Data.h"

ThresholdData::ThresholdData(){};

ThresholdData::ThresholdData(uint16_t value1, uint16_t value2, uint16_t value3) : absoluted{value1}, normalized(normalizeValue(value1)), lowerLimit{value2}, range{value3}{}
ThresholdData::ThresholdData(float value1, uint16_t value2, uint16_t value3) : absoluted{absoluteValue(value1)}, normalized(value1), lowerLimit{value2}, range{value3}{}

// setter
void ThresholdData::setAbsolute(uint16_t value){
  absoluted.uint16 = value;
  normalized = normalizeValue(value);
}

void ThresholdData::setNormalize(float value){
  absoluted.uint16 = absoluteValue(value);
  normalized = value;
}

void ThresholdData::setAbsoluteBytes(uint8_t value0, uint8_t value1){
  absoluted.bytes[0] = value0;
  absoluted.bytes[1] = value1;
  normalized = normalizeValue(absoluted.uint16);
}

// getter
uint16_t ThresholdData::getAbsoluted(){
  return absoluted.uint16;
}
float ThresholdData::getNormalized(){
  return normalized;
}
uint8_t* ThresholdData::getAbsolutedBytes(){
  return absoluted.bytes;
}

// converter
float ThresholdData::normalizeValue(uint16_t value){
  float result = (float)(value - lowerLimit) / (float)(range);
  if(result > 1.00){
    result = 1.00;
  }
  else if(result < 0){
    result = 0;
  }
  return result;
}

uint16_t ThresholdData::absoluteValue(float value){
  return (uint16_t)(value * (float)range) + lowerLimit;
}

ThresholdData& ThresholdData::operator=(const ThresholdData& other){
  if(this != &other){
    absoluted = other.absoluted;
    normalized = other.normalized;
    lowerLimit = other.lowerLimit;
    range = other.range;
  }
  return *this;
}