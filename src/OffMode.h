#ifndef OFF_MODE_H
#define OFF_MODE_H

#include "LightingMode.h"

class OffMode : public LightingMode {
public:
    OffMode(Adafruit_NeoPixel* strip, float* globalParam);
    void update() override;
    void reset() override;
};

#endif // OFF_MODE_H