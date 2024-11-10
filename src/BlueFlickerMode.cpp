#include "BlueFlickerMode.h"
#include "Utils.h"
#include <math.h>

BlueFlickerMode::BlueFlickerMode(Adafruit_NeoPixel* strip, float* globalParam) 
    : LightingMode(strip, globalParam) {
    // Initialisation des variables spécifiques au mode scintillement bleu
    minLedSpeed = 0.00005;
    maxLedSpeed = 0.0005;
    minLedSpeedChangeInterval = 2000;
    maxLedSpeedChangeInterval = 5000;
    hueMin = 32768;   // 180 degrés (cyan)
    hueMax = 49152;   // 270 degrés (violet)
    intensityMin = 0.01;
    intensityMax = 0.1;

    unsigned long initialTime = millis();
    for (int i = 0; i < NUM_LEDS_FLICKER; i++) {
        ledForce[i] = randomFloat(0.0, 1.0);
        ledSpeed[i] = randomFloat(minLedSpeed, maxLedSpeed);
        ledDirection[i] = (random(0, 2) == 0) ? -1 : 1;
        nextLedSpeedChange[i] = initialTime + random(minLedSpeedChangeInterval, maxLedSpeedChangeInterval);
        lastUpdateFlicker[i] = initialTime;
    }
}

void BlueFlickerMode::update() {
    unsigned long currentMillis = millis();

    for (int i = 0; i < NUM_LEDS_FLICKER; i++) {
        // Calculer le temps écoulé depuis la dernière mise à jour de cette LED
        unsigned long deltaTime = currentMillis - lastUpdateFlicker[i];
        lastUpdateFlicker[i] = currentMillis;

        // Mettre à jour la force de la LED en fonction de la vitesse et de la direction
        ledForce[i] += ledSpeed[i] * deltaTime * ledDirection[i];

        // Inverser la direction si la force atteint les bornes
        if (ledForce[i] >= 1.0) {
            ledForce[i] = 1.0;
            ledDirection[i] = -1;
        } else if (ledForce[i] <= 0.0) {
            ledForce[i] = 0.0;
            ledDirection[i] = 1;
        }

        // Vérifier s'il est temps de changer la vitesse de variation pour cette LED
        if (currentMillis >= nextLedSpeedChange[i]) {
            ledSpeed[i] = randomFloat(minLedSpeed, maxLedSpeed);
            nextLedSpeedChange[i] = currentMillis + random(minLedSpeedChangeInterval, maxLedSpeedChangeInterval);
        }

        // Calculer la teinte en fonction de la force
        uint16_t hue = hueMin + (uint16_t)((float)(hueMax - hueMin) * ledForce[i]);

        // Calculer l'intensité en fonction de la force et du globalParameter
        float intensity = intensityMin + (intensityMax - intensityMin) * ledForce[i] * (*globalParameter / 100.0);

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

void BlueFlickerMode::reset() {
    // Réinitialiser les variables si nécessaire
    // Aucun paramètre spécifique à réinitialiser pour le mode BlueFlicker
}