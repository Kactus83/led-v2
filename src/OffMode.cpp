#include "OffMode.h"

OffMode::OffMode(Adafruit_NeoPixel* strip, float* globalParam) 
    : LightingMode(strip, globalParam) {}

void OffMode::update() {
    // Éteindre toutes les LEDs
    leds->clear();
    leds->show();
}

void OffMode::reset() {
    // Rien à réinitialiser pour le mode Off
}