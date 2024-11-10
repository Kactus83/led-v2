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
    unsigned long lastUpdateFlicker[NUM_LEDS_FLICKER];

    // Nouvelles variables pour l'oscillation aléatoire
    float ledMinForce[NUM_LEDS_FLICKER];
    float ledMaxForce[NUM_LEDS_FLICKER];

    // Paramètres pour le mode scintillement bleu
    float minLedSpeed;
    float maxLedSpeed;
    float minLedForce;
    float maxLedForce;
    uint16_t hueMin;
    uint16_t hueMax;
    float intensityMin;
    float intensityMax;

    // Variables pour la conversion non linéaire de l'intensité
    float intensityExponent;

    // Variables pour le mode étoile
    bool isStarMode[NUM_LEDS_FLICKER];
    int maxStars;
    float starProbability;
    unsigned long starStartTime[NUM_LEDS_FLICKER];
    unsigned long starDuration[NUM_LEDS_FLICKER];
    float starMinIntensityStart;
    float starMaxIntensityStart;
    float starMinIntensityEnd;
    float starMaxIntensityEnd;
    unsigned long starMinRiseTime;
    unsigned long starMaxRiseTime;
    unsigned long starMinFallTime;
    unsigned long starMaxFallTime;
    float starCurrentIntensity[NUM_LEDS_FLICKER];
    unsigned long starRiseTime[NUM_LEDS_FLICKER];
    unsigned long starFallTime[NUM_LEDS_FLICKER];

    // Fonction pour mettre à jour les LEDs en mode étoile
    void updateStarMode(int index, unsigned long currentMillis);
};

#endif // BLUE_FLICKER_MODE_H