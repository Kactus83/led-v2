#include "WhiteMode.h"
#include "Utils.h"

WhiteMode::WhiteMode(Adafruit_NeoPixel* strip, float* globalParam) 
    : LightingMode(strip, globalParam) {}

void WhiteMode::update() {
    // Utiliser le paramètre global pour ajuster l'intensité (0 à 100)
    uint8_t brightness = mapf(*globalParameter, 0.0, 100.0, 0, 255);

    // Allumer toutes les LED en blanc avec l'intensité définie
    for (int i = 0; i < leds->numPixels(); i++) {
        leds->setPixelColor(i, leds->Color(brightness, brightness, brightness));
    }

    leds->show();
}

void WhiteMode::reset() {
    // Rien à réinitialiser pour le mode White
}