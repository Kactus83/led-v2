#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include <Arduino.h>

// Énumération pour les événements du bouton
enum class ButtonEvent {
    None,
    ShortPress,
    LongPressStart,
    LongPressEnd
};

class ButtonHandler {
public:
    ButtonHandler(uint8_t pin);
    void begin();
    ButtonEvent update();

private:
    uint8_t buttonPin;
    bool buttonState;
    bool lastButtonState;
    unsigned long lastDebounceTime;
    unsigned long buttonPressedTime;
    const unsigned long debounceDelay = 50;
    const unsigned long longPressTime = 800;
    bool isLongPress;
    bool longPressActive;
};

#endif // BUTTON_HANDLER_H