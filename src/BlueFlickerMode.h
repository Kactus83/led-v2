#ifndef BLUE_FLICKER_MODE_H
#define BLUE_FLICKER_MODE_H

#include "LightingMode.h"

class BlueFlickerMode : public LightingMode {
public:
    BlueFlickerMode(Adafruit_NeoPixel* strip, float* globalParam);
    void update() override;
    void reset() override;

private:
    // Variables spécifiques au mode scintillement bleu
    static const int NUM_LEDS_FLICKER = 10;
    float ledForce[NUM_LEDS_FLICKER];
    float ledSpeed[NUM_LEDS_FLICKER];
    int ledDirection[NUM_LEDS_FLICKER];
    unsigned long nextLedSpeedChange[NUM_LEDS_FLICKER];
    unsigned long lastUpdateFlicker[NUM_LEDS_FLICKER];

    // Paramètres pour le mode scintillement bleu
    float minLedSpeed;
    float maxLedSpeed;
    unsigned long minLedSpeedChangeInterval;
    unsigned long maxLedSpeedChangeInterval;
    uint16_t hueMin;
    uint16_t hueMax;
    float intensityMin;
    float intensityMax;

    void setLEDColorBlueFlicker(int index, float force);
};

#endif // BLUE_FLICKER_MODE_H