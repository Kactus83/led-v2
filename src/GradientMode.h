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
    int movementRangeStart;              // Index de début du mouvement de la LED maître
    int movementRangeEnd;                // Index de fin du mouvement de la LED maître
    float movementRangePercentage;       // Pourcentage du nombre total de LEDs pour le mouvement

    // Variables pour l'intensité
    float masterLedIntensity;            // Intensité de la LED maître (calculée dynamiquement)
    float masterLedIntensityMin;         // Intensité minimale de la LED maître
    float masterLedIntensityMax;         // Intensité maximale de la LED maître
    float minIntensity;                  // Intensité minimale des LEDs les plus éloignées (calculée dynamiquement)
    float minIntensityLow;               // Intensité minimale basse
    float minIntensityHigh;              // Intensité minimale haute
    float intensityCurveExponent;        // Exposant pour la courbe de décroissance de l'intensité (influencé par globalParameter)

    // Variables pour le gradient de couleur dynamique
    uint16_t hueCenter;                  // Teinte centrale du spread (0-65535)
    float hueSpread;                     // Largeur du spread de teinte (0.0-1.0, fraction de la plage de teinte totale)

    // Paramètres dynamiques pour le spread de teinte
    float hueSpreadMin;                  // Largeur minimale du spread de teinte
    float hueSpreadMax;                  // Largeur maximale du spread de teinte
    float hueSpreadSpeed;                // Vitesse d'évolution de la largeur du spread (cycles par seconde)
    uint16_t hueCenterMin;               // Teinte minimale du centre du spread (0-65535)
    uint16_t hueCenterMax;               // Teinte maximale du centre du spread (0-65535)
    float hueCenterSpeed;                // Vitesse d'évolution du centre du spread (cycles par seconde)

    // Variables de phase pour le spread de teinte
    float hueSpreadPhase;                // Phase actuelle pour la largeur du spread
    float hueCenterPhase;                // Phase actuelle pour le centre du spread

    // Variables pour l'influence du globalParameter
    float saturationLow;                 // Saturation basse
    float saturationHigh;                // Saturation haute
    uint8_t saturation;                  // Saturation actuelle (calculée dynamiquement)

    // Fonctions pour calculer l'intensité et la couleur
    float calculateIntensity(int ledIndex);
    uint32_t calculateColor(int ledIndex, float intensity);
};

#endif // GRADIENT_MODE_H