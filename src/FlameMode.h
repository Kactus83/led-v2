#ifndef FLAME_MODE_H
#define FLAME_MODE_H

#include "LightingMode.h"

class FlameMode : public LightingMode {
public:
    FlameMode(Adafruit_NeoPixel* strip, float* globalParam);
    void update() override;
    void reset() override;

private:
    // Variables spécifiques au mode flamme
    float globalStrength;
    float strengthIncrement;
    float minStrengthIncrement;
    float maxStrengthIncrement;
    unsigned long nextStrengthChange;
    unsigned long minStrengthChangeInterval;
    unsigned long maxStrengthChangeInterval;

    // Variables pour le minimum et le maximum de la force globale
    float globalForceMin;
    float globalForceMax;
    unsigned long nextForceRangeChange;
    unsigned long minForceRangeChangeInterval;
    unsigned long maxForceRangeChangeInterval;

    // Paramètres pour la courbe de force des LEDs
    float forceCurveExponent;
    float orangeZoneStart;
    float orangeZoneEnd;

    // Variables pour le timing
    unsigned long previousMillis;
    unsigned long currentMillis;
    const long interval = 50;   // Intervalle de mise à jour en millisecondes

    void scheduleNextStrengthChange();
    void scheduleNextForceRangeChange();
    void setLEDColorFlame(int index, float force);
};

#endif // FLAME_MODE_H