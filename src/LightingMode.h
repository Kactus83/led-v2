#ifndef LIGHTING_MODE_H
#define LIGHTING_MODE_H

#include <Adafruit_NeoPixel.h>

class LightingMode {
public:
    LightingMode(Adafruit_NeoPixel* strip, float* globalParam) 
        : leds(strip), globalParameter(globalParam) {}
    
    virtual void update() = 0;
    virtual void reset() = 0;

protected:
    Adafruit_NeoPixel* leds;
    float* globalParameter;
};

#endif // LIGHTING_MODE_H