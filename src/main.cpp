#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

// Inclusion des fichiers d’en-tête
#include "LightingMode.h"
#include "OffMode.h"
#include "WhiteMode.h"
#include "BlueFlickerMode.h"
#include "FlameMode.h"
#include "GradientMode.h"
#include "ButtonHandler.h"
#include "Utils.h"

// Définition des broches et paramètres généraux
#define PIN        6        // Pin de contrôle des LED
#define NUM_LEDS   10       // Nombre de LED
#define BUTTON_PIN 2        // Broche du bouton

// Création de l'objet NeoPixel
Adafruit_NeoPixel leds(NUM_LEDS, PIN, NEO_GRB + NEO_KHZ800);

// Gestionnaire de bouton
ButtonHandler buttonHandler(BUTTON_PIN);

// Paramètre global ajustable
float globalParameter = 50.0; // Initialisé à 50

// Variables pour l'ajustement du paramètre global
bool isAdjustingParameter = false;
unsigned long parameterLastUpdateTime = 0;
float baseSpeed = 0.75;
float exponentialFactor = 1.5;
unsigned long buttonPressedTime = 0;

// Tableau des modes d'éclairage
const int totalModes = 5; // Augmenté à 5 pour inclure le nouveau mode
LightingMode* modes[totalModes];
int currentModeIndex = 1; // Initialisé à 1 (blanc)

// Fonction pour mettre à jour le paramètre global
void updateGlobalParameter();

void setup() {
    // Initialisation des LED
    leds.begin();
    leds.show(); // Initialise toutes les LED à 'off'

    // Initialisation du port série pour le débogage
    Serial.begin(9600);

    // Initialisation des modes d'éclairage
    modes[0] = new OffMode(&leds, &globalParameter);
    modes[1] = new WhiteMode(&leds, &globalParameter);
    modes[2] = new BlueFlickerMode(&leds, &globalParameter);
    modes[3] = new FlameMode(&leds, &globalParameter);
    modes[4] = new GradientMode(&leds, &globalParameter);

    // Initialisation du gestionnaire de bouton
    buttonHandler.begin();

    // Réinitialiser le mode actuel
    modes[currentModeIndex]->reset();
}

void loop() {
    // Gestion du bouton
    ButtonEvent event = buttonHandler.update();

    if (event == ButtonEvent::ShortPress) {
        // Changement de mode sur appui court
        currentModeIndex = (currentModeIndex + 1) % totalModes;
        Serial.print("Changement de mode : ");
        Serial.println(currentModeIndex);
        modes[currentModeIndex]->reset();
    } else if (event == ButtonEvent::LongPressStart) {
        // Début de l'ajustement du paramètre global
        isAdjustingParameter = true;
        parameterLastUpdateTime = millis();
        buttonPressedTime = millis();
        Serial.println("Début de l'ajustement du paramètre global");
    } else if (event == ButtonEvent::LongPressEnd) {
        // Fin de l'ajustement du paramètre global
        isAdjustingParameter = false;
        Serial.println("Fin de l'ajustement du paramètre global");
    }

    // Mise à jour du paramètre global si en ajustement
    if (isAdjustingParameter) {
        updateGlobalParameter();
    }

    // Mise à jour du mode actuel
    modes[currentModeIndex]->update();
}

// Fonction pour mettre à jour le paramètre global
void updateGlobalParameter() {
    // Calculer le temps écoulé depuis la dernière mise à jour
    unsigned long currentTime = millis();
    unsigned long elapsedTime = currentTime - parameterLastUpdateTime;
    parameterLastUpdateTime = currentTime;

    // Calculer le facteur de variation exponentielle en fonction de la durée de l'appui long
    unsigned long pressDuration = millis() - buttonPressedTime;
    float dynamicSpeed = baseSpeed * pow(exponentialFactor, pressDuration / 1000.0); // Variation exponentielle avec le temps d'appui

    // Mettre à jour le phase en fonction de la vitesse dynamique
    static float phase = 0.0;
    phase += dynamicSpeed * (elapsedTime / 1000.0); // Convertir le temps en secondes

    if (phase > TWO_PI) {
        phase -= TWO_PI;
    }

    // Calculer le paramètre entre 0 et 100
    globalParameter = (sin(phase) + 1.0) * 50.0; // sin() varie entre -1 et 1, donc (sin() + 1) varie entre 0 à 2, multiplié par 50 donne 0 à 100

    // Afficher la valeur du paramètre dans la console série
    Serial.print("Paramètre global ajusté à : ");
    Serial.println(globalParameter);
}
