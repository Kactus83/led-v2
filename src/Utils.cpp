#include "Utils.h"
#include <Arduino.h>

float randomFloat(float min, float max) {
    return min + ((float)random(0, 10000) / 10000.0) * (max - min);
}

float mapf(float x, float in_min, float in_max, float out_min, float out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}