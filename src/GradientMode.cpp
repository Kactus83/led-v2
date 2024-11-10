#include "GradientMode.h"
#include "Utils.h"
#include <math.h>
#include <Arduino.h>

GradientMode::GradientMode(Adafruit_NeoPixel* strip, float* globalParam)
    : LightingMode(strip, globalParam) {
    // Initialisation des variables

    // Mouvement de la LED maître
    movementRangePercentage = 0.8; // La LED maître se déplace sur 80% des LEDs
    movementRangeStart = (int)((1.0 - movementRangePercentage) / 2.0 * leds->numPixels());
    movementRangeEnd = leds->numPixels() - movementRangeStart - 1;
    masterLedIndex = movementRangeStart;
    masterLedDirection = 1; // Commence en avançant
    lastMoveTime = millis();
    moveInterval = 50; // Intervalle par défaut de 500 ms entre les mouvements

    // Intensité
    masterLedIntensityMin = 0.5; // Intensité minimale de la LED maître
    masterLedIntensityMax = 1.0; // Intensité maximale de la LED maître
    masterLedIntensity = masterLedIntensityMax; // Initialisation
    minIntensityLow = 0.001; // Intensité minimale basse
    minIntensityHigh = 0.01; // Intensité minimale haute
    minIntensity = minIntensityLow; // Initialisation
    intensityCurveExponent = 2.0; // Exposant pour la courbe de décroissance

    // Gradient de couleur dynamique
    hueCenter = 0;        // Rouge (0 degrés)
    hueSpread = 0.2;      // Fraction de la plage de teinte totale

    // Paramètres dynamiques pour le spread de teinte
    hueSpreadMin = 0.1;   // Largeur minimale du spread de teinte (fraction)
    hueSpreadMax = 0.3;   // Largeur maximale du spread de teinte (fraction)
    hueSpreadSpeed = 0.001; // Vitesse d'évolution de la largeur du spread (cycles par seconde)
    hueCenterMin = 0;     // Teinte minimale du centre du spread (0-65535)
    hueCenterMax = 65535; // Teinte maximale du centre du spread (0-65535)
    hueCenterSpeed = 0.001; // Vitesse d'évolution du centre du spread (cycles par seconde)

    // Variables de phase pour le spread de teinte
    hueSpreadPhase = 0.0;
    hueCenterPhase = 0.0;

    // Variables pour l'influence du globalParameter
    saturationLow = 200;
    saturationHigh = 255;
    saturation = saturationHigh; // Initialisation
}

void GradientMode::update() {
    unsigned long currentTime = millis();

    // Calcul du temps écoulé depuis la dernière mise à jour
    unsigned long elapsedTime = currentTime - lastMoveTime;

    // Mise à jour des phases pour le spread de teinte
    float deltaTimeSeconds = (float)elapsedTime / 1000.0;
    hueSpreadPhase += hueSpreadSpeed * deltaTimeSeconds * TWO_PI; // Convert cycles/s to radians
    if (hueSpreadPhase > TWO_PI) {
        hueSpreadPhase -= TWO_PI;
    }

    hueCenterPhase += hueCenterSpeed * deltaTimeSeconds * TWO_PI;
    if (hueCenterPhase > TWO_PI) {
        hueCenterPhase -= TWO_PI;
    }

    // Calculer les valeurs actuelles de spread et de center
    float currentHueSpreadFraction = hueSpreadMin + (hueSpreadMax - hueSpreadMin) * (sin(hueSpreadPhase) + 1.0) / 2.0;
    float currentHueCenter = hueCenterMin + (float)(hueCenterMax - hueCenterMin) * (sin(hueCenterPhase) + 1.0) / 2.0;

    // Mise à jour du spread
    hueSpread = currentHueSpreadFraction; // Fraction de la plage de teinte totale (0.0 - 1.0)
    hueCenter = (uint16_t)currentHueCenter; // Teinte centrale actuelle

    // Mise à jour des paramètres en fonction du globalParameter
    // Ajuster moveInterval et intensityCurveExponent
    moveInterval = mapf(*globalParameter, 0.0, 100.0, 1000.0, 100.0); // De 1000 ms à 100 ms
    intensityCurveExponent = mapf(*globalParameter, 0.0, 100.0, 2.0, 4.0); // De 2.0 à 4.0

    // Influence du globalParameter sur les intensités et la saturation
    masterLedIntensity = mapf(*globalParameter, 0.0, 100.0, masterLedIntensityMin, masterLedIntensityMax);
    minIntensity = mapf(*globalParameter, 0.0, 100.0, minIntensityLow, minIntensityHigh);
    saturation = (uint8_t)mapf(*globalParameter, 0.0, 100.0, saturationLow, saturationHigh);

    // Mise à jour du mouvement de la LED maître
    if (elapsedTime >= moveInterval) {
        // Déplacer la LED maître
        masterLedIndex += masterLedDirection;

        // Vérifier les limites et inverser la direction si nécessaire
        if (masterLedIndex >= movementRangeEnd) {
            masterLedIndex = movementRangeEnd;
            masterLedDirection = -1; // Inverser la direction vers l'arrière
        } else if (masterLedIndex <= movementRangeStart) {
            masterLedIndex = movementRangeStart;
            masterLedDirection = 1; // Inverser la direction vers l'avant
        }

        // Réinitialiser lastMoveTime
        lastMoveTime = currentTime;
    }

    // Mise à jour des LEDs
    for (int i = 0; i < leds->numPixels(); i++) {
        // Calculer l'intensité en fonction de la distance à la LED maître
        float intensity = calculateIntensity(i);

        // Calculer la couleur en fonction de la distance et de l'intensité
        uint32_t color = calculateColor(i, intensity);

        leds->setPixelColor(i, color);
    }

    leds->show();
}

void GradientMode::reset() {
    // Réinitialiser les variables si nécessaire
    movementRangeStart = (int)((1.0 - movementRangePercentage) / 2.0 * leds->numPixels());
    movementRangeEnd = leds->numPixels() - movementRangeStart - 1;
    masterLedIndex = movementRangeStart;
    masterLedDirection = 1;
    lastMoveTime = millis();

    // Réinitialiser les phases
    hueSpreadPhase = 0.0;
    hueCenterPhase = 0.0;
}

float GradientMode::calculateIntensity(int ledIndex) {
    int distance = abs(ledIndex - masterLedIndex);
    int maxDistance = max(abs(movementRangeEnd - movementRangeStart), 1); // Éviter la division par zéro
    float normalizedDistance = (float)distance / (float)maxDistance;

    // Calculer l'intensité en utilisant l'exposant de courbe
    float intensity = masterLedIntensity * pow(1.0 - normalizedDistance, intensityCurveExponent);
    intensity = max(intensity, minIntensity);

    // Limiter l'intensité entre minIntensity et masterLedIntensity
    intensity = constrain(intensity, minIntensity, masterLedIntensity);

    return intensity;
}

uint32_t GradientMode::calculateColor(int ledIndex, float intensity) {
    int distance = abs(ledIndex - masterLedIndex);
    int maxDistance = max(abs(movementRangeEnd - movementRangeStart), 1);

    // Calculer la fraction de distance
    float t = (float)distance / (float)maxDistance;

    // Calculer la teinte en fonction de la distance et du spread
    float hueVariation = hueSpread * 65535.0; // Convertir la fraction de spread en unités de teinte
    uint16_t hueStartLocal = (uint16_t)(hueCenter - (hueVariation / 2.0));
    uint16_t hueEndLocal = (uint16_t)(hueCenter + (hueVariation / 2.0));

    // Assurer que hueStart et hueEnd sont dans [0, 65535] en utilisant modulo
    hueStartLocal = hueStartLocal % 65536;
    hueEndLocal = hueEndLocal % 65536;

    // Calculer la teinte interpolée
    uint16_t hue;
    if (hueEndLocal >= hueStartLocal) {
        hue = hueStartLocal + (uint16_t)(t * (hueEndLocal - hueStartLocal));
    } else {
        // Si hueEndLocal < hueStartLocal, cela signifie qu'il y a un wrap autour de 65535
        hue = (hueStartLocal + (uint16_t)(t * (65535 - hueStartLocal + hueEndLocal))) % 65536;
    }

    // Ajuster la valeur (brightness) en fonction de l'intensité
    uint8_t adjustedValue = (uint8_t)(255.0 * intensity);

    // Retourner la couleur
    return leds->ColorHSV(hue, saturation, adjustedValue);
}