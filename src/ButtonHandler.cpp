#include "ButtonHandler.h"

ButtonHandler::ButtonHandler(uint8_t pin) {
    buttonPin = pin;
    buttonState = HIGH;
    lastButtonState = HIGH;
    lastDebounceTime = 0;
    buttonPressedTime = 0;
    isLongPress = false;
    longPressActive = false;
}

void ButtonHandler::begin() {
    pinMode(buttonPin, INPUT_PULLUP);
}

ButtonEvent ButtonHandler::update() {
    ButtonEvent event = ButtonEvent::None;
    bool reading = digitalRead(buttonPin);

    // Vérifier si l'état du bouton a changé (brut)
    if (reading != lastButtonState) {
        lastDebounceTime = millis();
    }

    // Si le temps de debounce est écoulé
    if ((millis() - lastDebounceTime) > debounceDelay) {
        // Si l'état stable du bouton a changé
        if (reading != buttonState) {
            buttonState = reading;

            if (buttonState == LOW) {
                // Bouton vient d'être pressé
                buttonPressedTime = millis();
                isLongPress = false;
                longPressActive = false;
                Serial.println("Bouton pressé");
            } else {
                // Bouton vient d'être relâché
                unsigned long pressDuration = millis() - buttonPressedTime;

                if (pressDuration >= longPressTime) {
                    // Fin de l'appui long
                    isLongPress = false;
                    longPressActive = false;
                    Serial.println("Fin de l'appui long");
                    event = ButtonEvent::LongPressEnd;
                } else {
                    // Appui court détecté
                    if (!isLongPress) {
                        Serial.println("Appui court détecté");
                        event = ButtonEvent::ShortPress;
                    }
                }
            }
        }

        // Détection de l'appui long
        if (buttonState == LOW && !isLongPress) {
            unsigned long pressDuration = millis() - buttonPressedTime;

            if (pressDuration >= longPressTime) {
                // Appui long détecté
                isLongPress = true;
                longPressActive = true;
                Serial.println("Appui long détecté (début de l'ajustement du paramètre)");
                event = ButtonEvent::LongPressStart;
            }
        }
    }

    lastButtonState = reading;
    return event;
}