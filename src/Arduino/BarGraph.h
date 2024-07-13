#ifndef BARGRAPH_H
#define BARGRAPH_H

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

class BarGraph {
public:
  struct Bar {
    float currentValue = 0.0;
    float upperThreshold = 0.0;
    float lowerThreshold = 1.0;
  };

  BarGraph(int width, int height, int numBars);
  ~BarGraph();

  void setBarValue(int index, float value);
  void setBarThresholds(int index, float lower, float upper);
  void draw(Adafruit_SSD1306& display);

  int cursor = 0;

private:
  int screenWidth;
  int screenHeight;
  int numBars;
  Bar* bars;
};

#endif