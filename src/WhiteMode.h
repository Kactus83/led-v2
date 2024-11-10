#ifndef WHITE_MODE_H
#define WHITE_MODE_H

#include "LightingMode.h"

class WhiteMode : public LightingMode {
public:
    WhiteMode(Adafruit_NeoPixel* strip, float* globalParam);
    void update() override;
    void reset() override;
};

#endif // WHITE_MODE_H