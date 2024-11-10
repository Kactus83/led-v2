#ifndef GRADIENT_MODE_H
#define GRADIENT_MODE_H

#include "LightingMode.h"

class GradientMode : public LightingMode {
public:
    GradientMode(Adafruit_NeoPixel* strip, float* globalParam);
    void update() override;
    void reset() override;

private:
    // Variables pour le mouvement de la LED maître
    int masterLedIndex;                  // Index actuel de la LED maître
    int masterLedDirection;              // Direction du mouvement : 1 pour avant, -1 pour arrière
    unsigned long lastMoveTime;          // Dernière fois que la LED maître a bougé
    unsigned long moveInterval;          // Intervalle entre les mouvements (en millisecondes)

    // Variables pour l'intensité
    float maxIntensity;                  // Intensité maximale à la LED maître
    float minIntensity;                  // Intensité minimale aux LEDs les plus éloignées
    float intensityCurveExponent;        // Exposant pour la courbe de décroissance de l'intensité

    // Variables pour le gradient de couleur
    uint16_t hueStart;                   // Teinte de départ à la LED maître (0-65535)
    uint16_t hueEnd;                     // Teinte de fin aux LEDs les plus éloignées (0-65535)
    uint8_t saturation;                  // Saturation (0-255)

    // Fonctions pour calculer l'intensité et la couleur
    float calculateIntensity(int ledIndex);
    uint32_t calculateColor(int ledIndex, float intensity);
};

#endif // GRADIENT_MODE_H