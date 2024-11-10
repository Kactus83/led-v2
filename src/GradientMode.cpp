#include "GradientMode.h"
#include "Utils.h"
#include <Arduino.h>
#include <math.h>

GradientMode::GradientMode(Adafruit_NeoPixel* strip, float* globalParam)
    : LightingMode(strip, globalParam) {
    // Initialisation des variables

    // Mouvement de la LED maître
    masterLedIndex = 0;
    masterLedDirection = 1; // Commence en avançant
    lastMoveTime = millis();
    moveInterval = 500; // Intervalle par défaut de 500 ms entre les mouvements

    // Intensité
    maxIntensity = 1.0; // Intensité maximale à la LED maître
    minIntensity = 0.1; // Intensité minimale aux LEDs les plus éloignées
    intensityCurveExponent = 2.0; // Exposant pour la courbe de décroissance

    // Gradient de couleur
    hueStart = 0;        // Rouge (0 degrés)
    hueEnd = 43690;      // Bleu (240 degrés)
    saturation = 255;    // Saturation maximale
}

void GradientMode::update() {
    unsigned long currentTime = millis();

    // Mise à jour des paramètres en fonction du globalParameter
    // Par exemple, ajuster moveInterval et intensityCurveExponent
    moveInterval = mapf(*globalParameter, 0.0, 100.0, 1000, 100); // De 1000 ms à 100 ms
    intensityCurveExponent = mapf(*globalParameter, 0.0, 100.0, 1.0, 3.0); // De 1.0 à 3.0

    // Mise à jour du mouvement de la LED maître
    if (currentTime - lastMoveTime >= moveInterval) {
        lastMoveTime = currentTime;

        // Déplacer la LED maître
        masterLedIndex += masterLedDirection;

        // Vérifier les limites et inverser la direction si nécessaire
        if (masterLedIndex >= leds->numPixels() - 1) {
            masterLedIndex = leds->numPixels() - 1;
            masterLedDirection = -1; // Inverser la direction vers l'arrière
        } else if (masterLedIndex <= 0) {
            masterLedIndex = 0;
            masterLedDirection = 1; // Inverser la direction vers l'avant
        }
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
    masterLedIndex = 0;
    masterLedDirection = 1;
    lastMoveTime = millis();
}

float GradientMode::calculateIntensity(int ledIndex) {
    int distance = abs(ledIndex - masterLedIndex);
    int maxDistance = leds->numPixels() - 1; // Distance maximale possible
    float normalizedDistance = (float)distance / maxDistance;

    // Calculer l'intensité en utilisant l'exposant de courbe
    float intensity = maxIntensity - pow(normalizedDistance, intensityCurveExponent) * (maxIntensity - minIntensity);

    // Limiter l'intensité entre minIntensity et maxIntensity
    intensity = constrain(intensity, minIntensity, maxIntensity);

    return intensity;
}

uint32_t GradientMode::calculateColor(int ledIndex, float intensity) {
    int distance = abs(ledIndex - masterLedIndex);
    int maxDistance = leds->numPixels() - 1; // Distance maximale possible
    float t = (float)distance / maxDistance;

    // Calculer la teinte en fonction de la distance
    uint16_t hue = hueStart + (uint16_t)((float)(hueEnd - hueStart) * t);

    // Ajuster la valeur (brightness) en fonction de l'intensité
    uint8_t adjustedValue = (uint8_t)(255 * intensity);

    // Retourner la couleur
    return leds->ColorHSV(hue, saturation, adjustedValue);
}