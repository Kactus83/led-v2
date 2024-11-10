#include "FlameMode.h"
#include "Utils.h"
#include <math.h>

FlameMode::FlameMode(Adafruit_NeoPixel* strip, float* globalParam) 
    : LightingMode(strip, globalParam) {
    // Initialisation des variables spécifiques au mode flamme
    globalStrength = 0.5;
    strengthIncrement = 0.25;
    minStrengthIncrement = 0.05;
    maxStrengthIncrement = 0.15;
    nextStrengthChange = 0;
    minStrengthChangeInterval = 1000;
    maxStrengthChangeInterval = 3000;

    globalForceMin = 0.05;
    globalForceMax = 2.5;
    nextForceRangeChange = 0;
    minForceRangeChangeInterval = 2000;
    maxForceRangeChangeInterval = 5000;

    forceCurveExponent = 1.5;
    orangeZoneStart = 0.1;
    orangeZoneEnd = 0.9;

    previousMillis = 0;
    currentMillis = 0;

    scheduleNextStrengthChange();
    scheduleNextForceRangeChange();
}

void FlameMode::update() {
    currentMillis = millis();

    if (currentMillis - previousMillis >= (unsigned long)interval) {
        previousMillis = currentMillis;

        // Mettre à jour globalForceMax en fonction du globalParameter
        globalForceMax = mapf(*globalParameter, 0.0, 100.0, 0.2, 2.5);

        // Vérifier s'il est temps de changer les valeurs min et max de la force globale
        if (currentMillis >= nextForceRangeChange) {
            // Générer de nouvelles valeurs pour globalForceMin
            globalForceMin = randomFloat(0.1, 0.5);  // Valeurs min entre 0.1 et 0.5

            // S'assurer que globalForceMin est inférieur à globalForceMax
            if (globalForceMin >= globalForceMax - 0.1) {
                globalForceMin = globalForceMax - 0.1;
            }

            scheduleNextForceRangeChange();
        }

        // Mettre à jour la force globale
        globalStrength += strengthIncrement;
        if (globalStrength > TWO_PI) {
            globalStrength -= TWO_PI;
        }

        // Vérifier s'il est temps de changer la vitesse de variation
        if (currentMillis >= nextStrengthChange) {
            strengthIncrement = randomFloat(minStrengthIncrement, maxStrengthIncrement);
            scheduleNextStrengthChange();
        }

        // Calculer la force globale oscillant entre globalForceMin et globalForceMax
        float sinValue = (sin(globalStrength) + 1.0) / 2.0;  // Valeur entre 0 et 1
        float globalForce = globalForceMin + sinValue * (globalForceMax - globalForceMin);

        // Calculer la force pour chaque LED
        for (unsigned int i = 0; i < leds->numPixels(); i++) {
            float position = (float)i / (leds->numPixels() - 1);  // Position normalisée entre 0 et 1
            position = 1.0 - position;  // Inverser pour que la base soit à 0

            // Appliquer la courbe de force avec l'exposant
            float ledForce = pow(position, forceCurveExponent);

            // Appliquer la force globale
            ledForce *= globalForce;

            // Déterminer la couleur et l'intensité
            setLEDColorFlame(i, ledForce);
        }

        leds->show();
    }
}

void FlameMode::reset() {
    // Réinitialiser les variables si nécessaire
    // Aucun paramètre spécifique à réinitialiser pour le mode Flame
}

void FlameMode::scheduleNextStrengthChange() {
    unsigned long intervalRandom = random(minStrengthChangeInterval, maxStrengthChangeInterval);
    nextStrengthChange = currentMillis + intervalRandom;
}

void FlameMode::scheduleNextForceRangeChange() {
    unsigned long intervalRandom = random(minForceRangeChangeInterval, maxForceRangeChangeInterval);
    nextForceRangeChange = currentMillis + intervalRandom;
}

void FlameMode::setLEDColorFlame(int index, float force) {
    // Limiter la force entre 0 et 1
    force = constrain(force, 0.0, 1.0);

    // Calculer l'intensité (brightness)
    uint8_t brightness = (uint8_t)(force * 255);

    // Déterminer la couleur en fonction de la force avec les paramètres
    uint8_t r, g, b;

    if (force >= orangeZoneEnd) {
        // Zone chaude, proche du blanc
        r = 255;
        g = 255;
        b = (uint8_t)(200 * (1.0 - force));  // Légère teinte bleue diminuant avec la force
    } else if (force >= orangeZoneStart) {
        // Zone orange
        float ratio = (force - orangeZoneStart) / (orangeZoneEnd - orangeZoneStart);
        r = 255;
        g = (uint8_t)(150 * ratio);  // Augmente de 0 à 150
        b = 0;
    } else {
        // Zone rouge avec intensité décroissante
        r = (uint8_t)(255 * (force / orangeZoneStart));  // Diminue de 255 à 0
        g = 0;
        b = 0;
    }

    // Appliquer l'intensité
    r = (r * brightness) / 255;
    g = (g * brightness) / 255;
    b = (b * brightness) / 255;

    leds->setPixelColor(index, r, g, b);
}