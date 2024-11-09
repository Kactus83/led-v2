#include <Adafruit_NeoPixel.h>
#include <math.h>

#define PIN        6        // Pin de contrôle des LED
#define NUM_LEDS   10       // Nombre de LED
#define BUTTON_PIN 2        // Broche du bouton

// Définition des constantes pour le bouton
#define DEBOUNCE_DELAY 50     // Délai pour le debouncing en ms
#define LONG_PRESS_TIME 800   // Durée pour considérer un appui long en ms

Adafruit_NeoPixel leds(NUM_LEDS, PIN, NEO_GRB + NEO_KHZ800);

// Variables pour la gestion des modes
int currentMode = 0;          // Mode actuel (0 : flamme, 1 : blanc, 2 : scintillement bleu)
const int totalModes = 3;     // Nombre total de modes disponibles

float globalParameter = 50;    // Paramètre global (0 à 100), initialisé à 50 

// Variables pour le bouton
unsigned long lastDebounceTime = 0;
unsigned long buttonPressedTime = 0;
unsigned long buttonReleasedTime = 0;
bool buttonState = HIGH;          // État actuel du bouton
bool lastButtonState = HIGH;      // État précédent du bouton
bool isLongPress = false;

// Variables pour la variation du paramètre pendant l'appui long
float baseSpeed = 0.75;             // Vitesse de base de variation du paramètre
float exponentialFactor = 1.5;      // Facteur exponentiel pour accélérer la variation
unsigned long parameterLastUpdateTime = 0;
bool isAdjustingParameter = false; // Indique si on est en train d'ajuster le paramètre

/* ------------------- Déclaration des fonctions ------------------- */

// Fonctions pour la gestion du bouton
void handleButton();
void nextMode();
void updateGlobalParameter();

// Fonctions pour le mode flamme
void modeFlame();
void scheduleNextStrengthChange();
void scheduleNextForceRangeChange();
float randomFloat(float min, float max);
float mapf(float x, float in_min, float in_max, float out_min, float out_max);
void setLEDColorFlame(int index, float force);

// Fonction pour le mode blanc
void modeWhite();

// Fonction pour le mode scintillement bleu
void modeBlueFlicker();


/* ------------------- Fonctions utilitaires ------------------- */

float randomFloat(float min, float max) {
  return min + ((float)random(0, 10000) / 10000.0) * (max - min);
}

float mapf(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

/* ------------------- Variables pour le mode flamme ------------------- */

// Paramètres spécifiques au mode flamme
float globalStrength = 0.5;
float strengthIncrement = 0.25; // Vitesse de changement de la force globale
float minStrengthIncrement = 0.05;  // Vitesse minimale
float maxStrengthIncrement = 0.15;  // Vitesse maximale
unsigned long nextStrengthChange = 0;
unsigned long minStrengthChangeInterval = 1000; // Intervalle min pour changer la vitesse (en ms)
unsigned long maxStrengthChangeInterval = 3000; // Intervalle max pour changer la vitesse (en ms)

// Variables pour le minimum et le maximum de la force globale
float globalForceMin = 0.05;  // Valeur minimale initiale
float globalForceMax = 2.5;  // Valeur maximale initiale (sera mis à jour en fonction du globalParameter)
unsigned long nextForceRangeChange = 0;
unsigned long minForceRangeChangeInterval = 2000; // Intervalle min pour changer les valeurs min et max (en ms)
unsigned long maxForceRangeChangeInterval = 5000; // Intervalle max pour changer les valeurs min et max (en ms)

// Paramètres pour la courbe de force des LEDs
float forceCurveExponent = 1.5; // Plus la valeur est grande, plus la chute est rapide
float orangeZoneStart = 0.1;    // Force à partir de laquelle commence la zone orange
float orangeZoneEnd = 0.9;      // Force à laquelle se termine la zone orange

// Variables pour le timing
unsigned long previousMillis = 0;
unsigned long currentMillis = 0;
const long interval = 50;   // Intervalle de mise à jour en millisecondes

/* ------------------- Variables pour le mode scintillement bleu ------------------- */

// Paramètres pour le mode scintillement bleu
float ledForce[NUM_LEDS];         // Force pour chaque LED
float ledIncrement[NUM_LEDS];     // Vitesse de changement de force pour chaque LED
float ledPhase[NUM_LEDS];         // Phase initiale pour chaque LED
unsigned long nextLedChange[NUM_LEDS];  // Prochain temps de changement de vitesse pour chaque LED

float minLedIncrement = 0.001;     // Vitesse minimale
float maxLedIncrement = 0.01;      // Vitesse maximale
unsigned long minLedChangeInterval = 2000; // Intervalle min pour changer la vitesse (en ms)
unsigned long maxLedChangeInterval = 5000; // Intervalle max pour changer la vitesse (en ms)

float blueMin = 165.0;             // Valeur minimale du bleu (0-255)
float blueMax = 205.0;             // Valeur maximale du bleu (0-255)
float intensityMin = 0.005;         // Intensité minimale (0.0 - 1.0)
float intensityMax = 0.05;         // Intensité maximale (0.0 - 1.0)

/* ------------------- Fonction setup ------------------- */

void setup() {
  // Initialisation des LED
  leds.begin();
  leds.show(); // Initialise toutes les LED à 'off'

  // Initialisation du bouton
  pinMode(BUTTON_PIN, INPUT_PULLUP); // Utilise la résistance de pull-up interne

  // Initialisation du port série pour le débogage
  Serial.begin(9600);

  // Initialisation du random
  randomSeed(analogRead(0)); // Initialisation du générateur aléatoire

  // Planification initiale des changements pour le mode flamme
  scheduleNextStrengthChange();
  scheduleNextForceRangeChange();

  // Initialisation des variables pour le mode scintillement bleu
  for (int i = 0; i < NUM_LEDS; i++) {
    ledForce[i] = randomFloat(0.0, TWO_PI);
    ledIncrement[i] = randomFloat(minLedIncrement, maxLedIncrement);
    ledPhase[i] = randomFloat(0.0, TWO_PI);
    nextLedChange[i] = millis() + random(minLedChangeInterval, maxLedChangeInterval);
  }
}

/* ------------------- Fonction loop ------------------- */

void loop() {
  // Gestion du bouton
  handleButton();

  // Si on est en train d'ajuster le paramètre, mettre à jour le globalParameter
  if (isAdjustingParameter) {
    updateGlobalParameter();
  }

  // Appel de la fonction correspondant au mode actuel
  switch (currentMode) {
    case 0:
      modeFlame();
      break;
    case 1:
      modeWhite();
      break;
    case 2:
      modeBlueFlicker();
      break;
    default:
      // Mode par défaut si nécessaire
      break;
  }
}

/* ------------------- Gestion du bouton ------------------- */

void handleButton() {
  bool reading = digitalRead(BUTTON_PIN);

  // Vérifier si l'état du bouton a changé (brut)
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  // Si le temps de debounce est écoulé
  if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
    // Si l'état stable du bouton a changé
    if (reading != buttonState) {
      buttonState = reading;

      if (buttonState == LOW) {
        // Bouton vient d'être pressé
        buttonPressedTime = millis();
        isLongPress = false;
        isAdjustingParameter = false;
        Serial.println("Bouton pressé");
      } else {
        // Bouton vient d'être relâché
        unsigned long pressDuration = millis() - buttonPressedTime;

        if (pressDuration >= LONG_PRESS_TIME) {
          // Fin de l'appui long
          isAdjustingParameter = false;
          Serial.println("Fin de l'appui long");
        } else {
          // Appui court détecté
          if (!isLongPress) {
            Serial.println("Appui court détecté");
            nextMode();
          }
        }
      }
    }

    // Détection de l'appui long
    if (buttonState == LOW && !isLongPress) {
      unsigned long pressDuration = millis() - buttonPressedTime;

      if (pressDuration >= LONG_PRESS_TIME) {
        // Appui long détecté
        isLongPress = true;
        isAdjustingParameter = true;
        parameterLastUpdateTime = millis();
        Serial.println("Appui long détecté (début de l'ajustement du paramètre)");
      }
    }
  }

  lastButtonState = reading;
}

void nextMode() {
  currentMode = (currentMode + 1) % totalModes;
  Serial.print("Changement de mode : ");
  Serial.println(currentMode);
  // Réinitialiser les paramètres si nécessaire
}

void updateGlobalParameter() {
  // Calculer le temps écoulé depuis la dernière mise à jour
  unsigned long currentTime = millis();
  unsigned long elapsedTime = currentTime - parameterLastUpdateTime;
  parameterLastUpdateTime = currentTime;

  // Calculer le facteur de variation exponentielle en fonction de la durée de l'appui long
  unsigned long pressDuration = millis() - buttonPressedTime;
  float dynamicSpeed = baseSpeed * pow(exponentialFactor, pressDuration / 1000.0); // Variation exponentielle avec le temps d'appui

  // Mettre à jour la phase en fonction de la vitesse dynamique
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

/* ------------------- Mode 0 : Effet de flamme ------------------- */

void modeFlame() {
  currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Mettre à jour globalForceMax en fonction du globalParameter
    globalForceMax = mapf(globalParameter, 0.0, 100.0, 0.2, 2.5);

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
    for (int i = 0; i < NUM_LEDS; i++) {
      float position = (float)i / (NUM_LEDS - 1);  // Position normalisée entre 0 et 1
      position = 1.0 - position;  // Inverser pour que la base soit à 0

      // Appliquer la courbe de force avec l'exposant
      float ledForce = pow(position, forceCurveExponent);

      // Appliquer la force globale
      ledForce *= globalForce;

      // Déterminer la couleur et l'intensité
      setLEDColorFlame(i, ledForce);
    }

    leds.show();
  }
}

void scheduleNextStrengthChange() {
  unsigned long interval = random(minStrengthChangeInterval, maxStrengthChangeInterval);
  nextStrengthChange = currentMillis + interval;
}

void scheduleNextForceRangeChange() {
  unsigned long interval = random(minForceRangeChangeInterval, maxForceRangeChangeInterval);
  nextForceRangeChange = currentMillis + interval;
}

void setLEDColorFlame(int index, float force) {
  // Limiter la force entre 0 et 1
  if (force > 1.0) force = 1.0;
  if (force < 0.0) force = 0.0;

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

  leds.setPixelColor(index, r, g, b);
}

/* ------------------- Mode 1 : Lumière blanche ------------------- */

void modeWhite() {
  // Utiliser le paramètre global pour ajuster l'intensité (0 à 100)
  uint8_t brightness = map((int)globalParameter, 0, 100, 0, 255);

  // Allumer toutes les LED en blanc avec l'intensité définie
  for (int i = 0; i < NUM_LEDS; i++) {
    leds.setPixelColor(i, leds.Color(brightness, brightness, brightness));
  }

  leds.show();
}

/* ------------------- Mode 2 : Scintillement bleu amélioré ------------------- */

void modeBlueFlicker() {
  currentMillis = millis();

  // Mettre à jour chaque LED individuellement
  for (int i = 0; i < NUM_LEDS; i++) {
    // Mettre à jour la force de la LED
    ledForce[i] += ledIncrement[i];
    if (ledForce[i] > TWO_PI) {
      ledForce[i] -= TWO_PI;
    }

    // Vérifier s'il est temps de changer la vitesse de variation pour cette LED
    if (currentMillis >= nextLedChange[i]) {
      ledIncrement[i] = randomFloat(minLedIncrement, maxLedIncrement);
      nextLedChange[i] = currentMillis + random(minLedChangeInterval, maxLedChangeInterval);
    }

    // Calculer la force oscillant entre 0 et 1
    float sinValue = (sin(ledForce[i] + ledPhase[i]) + 1.0) / 2.0;  // Valeur entre 0 et 1

    // Ajuster l'intensité en fonction du globalParameter
    float intensityRange = intensityMax - intensityMin;
    float intensity = intensityMin + sinValue * intensityRange * (globalParameter / 100.0);

    // Calculer la valeur du bleu en fonction de la force
    float blueRange = blueMax - blueMin;
    uint8_t blueValue = (uint8_t)(blueMin + sinValue * blueRange);

    // Calculer les valeurs de rouge et vert pour la nuance de bleu souhaitée
    // Transition du violet (plus de rouge) au cyan (plus de vert)
    uint8_t r = (uint8_t)(intensity * (255 - sinValue * 255)); // Réduit le rouge avec la force
    uint8_t g = (uint8_t)(intensity * (sinValue * 255));       // Augmente le vert avec la force
    uint8_t b = (uint8_t)(blueValue * intensity);             // Applique l'intensité au bleu

    // Appliquer les couleurs aux LEDs
    leds.setPixelColor(i, r, g, b);
  }

  leds.show();
}

