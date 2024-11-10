#include "BlueFlickerMode.h"
#include "Utils.h"
#include <math.h>
#include <Arduino.h>

BlueFlickerMode::BlueFlickerMode(Adafruit_NeoPixel* strip, float* globalParam) 
    : LightingMode(strip, globalParam) {
    // Initialisation des variables spécifiques au mode scintillement bleu
    minLedSpeed = 0.0002;
    maxLedSpeed = 0.001;
    minLedForce = 0.0;
    maxLedForce = 1.0;
    hueMin = 34768;   // 180 degrés (cyan)
    hueMax = 51152;   // 270 degrés (violet)
    intensityMin = 0.0025;
    intensityMax = 1.0;
    intensityExponent = 3.0; // Exposant pour la conversion non linéaire

    // Variables pour le mode étoile
    maxStars = 2;             // Nombre maximum de LEDs en mode étoile simultanément
    starProbability = 0.00005; // Probabilité pour une LED d'entrer en mode étoile à chaque mise à jour
    starMinIntensityStart = 0.0;
    starMaxIntensityStart = 0.1;
    starMinIntensityEnd = 0.2;
    starMaxIntensityEnd = 1.0;
    starMinRiseTime = 50;    // Temps de montée minimum en ms
    starMaxRiseTime = 500;    // Temps de montée maximum en ms
    starMinFallTime = 100;    // Temps de descente minimum en ms
    starMaxFallTime = 1000;    // Temps de descente maximum en ms

    unsigned long initialTime = millis();
    for (int i = 0; i < NUM_LEDS_FLICKER; i++) {
        ledForce[i] = randomFloat(minLedForce, maxLedForce);
        ledSpeed[i] = randomFloat(minLedSpeed, maxLedSpeed);
        lastUpdateFlicker[i] = initialTime;

        // Initialisation des bornes de force individuelles
        ledMinForce[i] = randomFloat(0.0, 0.3);
        ledMaxForce[i] = randomFloat(0.7, 1.0);

        // Initialisation du mode étoile
        isStarMode[i] = false;
        starCurrentIntensity[i] = 0.0;
        starStartTime[i] = 0;
        starDuration[i] = 0;
        starRiseTimeActual[i] = 0;
        starFallTimeActual[i] = 0;
        starMaxIntensityEndActual[i] = 0.0;
    }
}

void BlueFlickerMode::update() {
    unsigned long currentMillis = millis();

    int currentStars = 0;
    // Compter le nombre de LEDs en mode étoile
    for (int i = 0; i < NUM_LEDS_FLICKER; i++) {
        if (isStarMode[i]) {
            currentStars++;
        }
    }

    // Calcul des variables dynamiques basées sur globalParameter
    float dynamicIntensityMax = mapf(*globalParameter, 0.0, 100.0, 0.2, 1.0);
    float dynamicIntensityExponent = mapf(*globalParameter, 0.0, 100.0, 2.0, 3.0);
    float dynamicStarProbability = mapf(*globalParameter, 0.0, 100.0, 0.00005, 0.000025);
    float dynamicStarMaxIntensityEnd = mapf(*globalParameter, 0.0, 100.0, 0.5, 1.0);

    // Mise à jour des variables dépendantes de globalParameter
    intensityMax = dynamicIntensityMax;
    intensityExponent = dynamicIntensityExponent;
    starProbability = dynamicStarProbability;
    // starMaxIntensityEndActual est géré par LED individuelle

    for (int i = 0; i < NUM_LEDS_FLICKER; i++) {
        // Gestion du mode étoile
        if (isStarMode[i]) {
            updateStarMode(i, currentMillis);
            continue; // Passer à la LED suivante
        } else if (currentStars < maxStars && randomFloat(0.0, 1.0) < starProbability) {
            // La LED entre en mode étoile
            isStarMode[i] = true;
            starStartTime[i] = currentMillis;
            starRiseTimeActual[i] = random(starMinRiseTime, starMaxRiseTime);
            starFallTimeActual[i] = random(starMinFallTime, starMaxFallTime);
            starDuration[i] = starRiseTimeActual[i] + starFallTimeActual[i];
            starCurrentIntensity[i] = starMinIntensityStart;

            // Intensités de début et de fin pour l'animation
            starMinIntensityStart = randomFloat(0.0, 0.1);
            starMaxIntensityEndActual[i] = randomFloat(0.5, 1.0) * (dynamicStarMaxIntensityEnd / 1.0); // Ajusté selon globalParameter

            currentStars++;
            continue; // Passer à la LED suivante
        }

        // Calculer le temps écoulé depuis la dernière mise à jour de cette LED
        unsigned long deltaTime = currentMillis - lastUpdateFlicker[i];
        lastUpdateFlicker[i] = currentMillis;

        // Mettre à jour la force de la LED en fonction de la vitesse et de la direction aléatoire
        ledForce[i] += ledSpeed[i] * deltaTime * ((random(0, 2) == 0) ? -1 : 1);

        // Limiter la force entre les bornes individuelles
        if (ledForce[i] > ledMaxForce[i]) {
            ledForce[i] = ledMaxForce[i];
        } else if (ledForce[i] < ledMinForce[i]) {
            ledForce[i] = ledMinForce[i];
        }

        // Changer de vitesse de manière aléatoire
        if (random(0, 1000) < 10) { // Probabilité de changer de vitesse
            ledSpeed[i] = randomFloat(minLedSpeed, maxLedSpeed);
        }

        // Calculer la teinte en fonction de la force
        uint16_t hue = hueMin + (uint16_t)((float)(hueMax - hueMin) * ledForce[i]);

        // Conversion non linéaire de l'intensité avec exponent
        float normalizedForce = (ledForce[i] - minLedForce) / (maxLedForce - minLedForce);
        float intensity = intensityMin + (intensityMax - intensityMin) * pow(normalizedForce, intensityExponent) * (*globalParameter / 100.0);

        // Limiter l'intensité entre 0 et 1
        intensity = constrain(intensity, 0.0, 1.0);

        uint8_t value = (uint8_t)(intensity * 255);

        // Obtenir la couleur HSV
        uint32_t color = leds->ColorHSV(hue, 255, value);

        // Appliquer la couleur à la LED
        leds->setPixelColor(i, color);
    }

    leds->show();
}

void BlueFlickerMode::updateStarMode(int index, unsigned long currentMillis) {
    unsigned long elapsedTime = currentMillis - starStartTime[index];

    if (elapsedTime < starRiseTimeActual[index]) {
        // Phase de montée
        float t = (float)elapsedTime / (float)starRiseTimeActual[index];
        starCurrentIntensity[index] = starMinIntensityStart + t * (starMaxIntensityEndActual[index] - starMinIntensityStart);
    } else if (elapsedTime < starDuration[index]) {
        // Phase de descente
        float t = (float)(elapsedTime - starRiseTimeActual[index]) / (float)starFallTimeActual[index];
        starCurrentIntensity[index] = starMaxIntensityEndActual[index] * (1.0 - t);
    } else {
        // Fin du mode étoile
        isStarMode[index] = false;
        starCurrentIntensity[index] = 0.0;
        return;
    }

    // Limiter l'intensité entre 0 et 1
    float intensity = constrain(starCurrentIntensity[index], 0.0, 1.0);

    uint8_t value = (uint8_t)(intensity * 255);

    // Couleur blanche
    leds->setPixelColor(index, value, value, value);
}

void BlueFlickerMode::reset() {
    // Réinitialiser les variables si nécessaire
    for (int i = 0; i < NUM_LEDS_FLICKER; i++) {
        isStarMode[i] = false;
        starCurrentIntensity[i] = 0.0;
    }
}