#include "BarGraph.h"

BarGraph::BarGraph(int width, int height, int numBars)
  : screenWidth(width), screenHeight(height), numBars(numBars) {
  bars = new Bar[numBars];
}

BarGraph::~BarGraph() {
  delete[] bars;
}

void BarGraph::setBarValue(int barIndex, float value) {
  if (barIndex >= 0 && barIndex < numBars) {
    //bars[barIndex].currentValue = 1.0 - value;
    bars[barIndex].currentValue = constrain(1.0 - value, 0.0, 1.0);
  }
}

void BarGraph::setBarThresholds(int barIndex, float lower, float upper) {
  if (barIndex >= 0 && barIndex < numBars) {
    bars[barIndex].lowerThreshold = lower;
    bars[barIndex].upperThreshold = upper;
  }
}

void BarGraph::draw(Adafruit_SSD1306& display) {
  int areaWidth = screenWidth / numBars;
  int barWidth = areaWidth * 2 / 3;
  int arrowWidth = areaWidth - barWidth;

  for (int i = 0; i < numBars; ++i) {
    int barHeight = bars[i].currentValue * screenHeight;
    int drawOffsetX = i * areaWidth;

    // 棒グラフの描画
    display.fillRect(drawOffsetX + arrowWidth, 0, barWidth - 1, screenHeight, SSD1306_WHITE);
    display.fillRect(drawOffsetX + arrowWidth + 1, 1, barWidth - 3, barHeight - 1, SSD1306_INVERSE);

    // 閾値の描画
    int upperY = 1 + (1.0 - bars[i].upperThreshold) * screenHeight;
    int lowerY = -1 + (1.0 - bars[i].lowerThreshold) * screenHeight;

    if (cursor != 0 && cursor / 2 == i && cursor % 2 == 1) {
      display.fillTriangle(drawOffsetX + 1, upperY, 
                           drawOffsetX + arrowWidth - 1, upperY,
                           drawOffsetX + 1, upperY - arrowWidth + 2, SSD1306_WHITE);
      display.drawTriangle(drawOffsetX + 1, lowerY, 
                         drawOffsetX + arrowWidth - 1, lowerY,
                         drawOffsetX + 1, lowerY + arrowWidth - 2, SSD1306_WHITE);
    }
    else if(cursor != 0 && cursor / 2 - 1 == i && cursor % 2 == 0) {
      display.drawTriangle(drawOffsetX + 1, upperY, 
                           drawOffsetX + arrowWidth - 1, upperY,
                           drawOffsetX + 1, upperY - arrowWidth + 2, SSD1306_WHITE);
      display.fillTriangle(drawOffsetX + 1, lowerY, 
                         drawOffsetX + arrowWidth - 1, lowerY,
                         drawOffsetX + 1, lowerY + arrowWidth - 2, SSD1306_WHITE);
    }
    else {
      display.drawTriangle(drawOffsetX + 1, upperY, 
                           drawOffsetX + arrowWidth - 1, upperY,
                           drawOffsetX + 1, upperY - arrowWidth + 2, SSD1306_WHITE);
      display.drawTriangle(drawOffsetX + 1, lowerY, 
                         drawOffsetX + arrowWidth - 1, lowerY,
                         drawOffsetX + 1, lowerY + arrowWidth - 2, SSD1306_WHITE);
    }
    display.drawLine(drawOffsetX + arrowWidth + 1, upperY, drawOffsetX + areaWidth - 3, upperY, SSD1306_INVERSE);
    display.drawLine(drawOffsetX + arrowWidth + 1, lowerY, drawOffsetX + areaWidth - 3, lowerY, SSD1306_INVERSE);
  }
}
