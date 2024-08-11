#include "Threshold_Data.h"

ThresholdData::ThresholdData(){};

ThresholdData::ThresholdData(uint16_t value1, uint16_t value2, uint16_t value3) : absoluted{value1}, normalized(normalizeValue(value1)), lowerLimit{value2}, range{value3}{}
ThresholdData::ThresholdData(float value1, uint16_t value2, uint16_t value3) : absoluted{absoluteValue(value1)}, normalized(value1), lowerLimit{value2}, range{value3}{}

// setter
void ThresholdData::setNormalize(float value){
  absoluted.uint16 = absoluteValue(value);
  normalized = value;
}

void ThresholdData::setAbsolute(uint16_t value){
  absoluted.uint16 = value;
  normalized = normalizeValue(value);
}

void ThresholdData::setLowerLimit(uint16_t lowerLimit_){
  lowerLimit.uint16 = lowerLimit_;
  normalized = normalizeValue(absoluted.uint16);
}

void ThresholdData::setRange(uint16_t range_){
  range.uint16 = range_;
  normalized = normalizeValue(absoluted.uint16);
}

void ThresholdData::setAbsoluteBytes(uint8_t value0, uint8_t value1){
  absoluted.bytes[0] = value0;
  absoluted.bytes[1] = value1;
  normalized = normalizeValue(absoluted.uint16);
}

void ThresholdData::setLowerLimitBytes(uint8_t lowerLimit0, uint8_t lowerLimit1){
  lowerLimit.bytes[0] = lowerLimit0;
  lowerLimit.bytes[1] = lowerLimit1;
  normalized = normalizeValue(absoluted.uint16);
}

void ThresholdData::setRangeBytes(uint8_t range0, uint8_t range1){
  range.bytes[0] = range0;
  range.bytes[1] = range1;
  normalized = normalizeValue(absoluted.uint16);
}


// getter
float ThresholdData::getNormalized(){
  return normalized;
}

uint16_t ThresholdData::getAbsoluted(){
  return absoluted.uint16;
}

uint16_t ThresholdData::getLowerLimit(){
  return lowerLimit.uint16;
}

uint16_t ThresholdData::getRange(){
  return range.uint16;
}

uint8_t* ThresholdData::getAbsolutedBytes(){
  return absoluted.bytes;
}

uint8_t* ThresholdData::getLowerLimitBytes(){
  return lowerLimit.bytes;
}

uint8_t* ThresholdData::getRangeBytes(){
  return range.bytes;
}

// converter
float ThresholdData::normalizeValue(uint16_t value){
  float result = (float)(value - lowerLimit.uint16) / (float)(range.uint16);
  if(result > 1.00){
    result = 1.00;
  }
  else if(result < 0){
    result = 0;
  }
  return result;
}

uint16_t ThresholdData::absoluteValue(float value){
  return (uint16_t)(value * (float)range.uint16) + lowerLimit.uint16;
}

ThresholdData& ThresholdData::operator=(const ThresholdData& other){
  if(this != &other){
    absoluted = other.absoluted;
    normalized = other.normalized;
    lowerLimit.uint16 = other.lowerLimit.uint16;
    range.uint16 = other.range.uint16;
  }
  return *this;
}
