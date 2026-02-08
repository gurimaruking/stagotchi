#include "input.h"

void InputManager::init() {
    // M5Unified handles button/touch mapping internally
}

void InputManager::update() {
    _pressed[0] = M5.BtnA.wasPressed();
    _pressed[1] = M5.BtnB.wasPressed();
    _pressed[2] = M5.BtnC.wasPressed();
    _held[0]    = M5.BtnA.pressedFor(800);
    _held[1]    = M5.BtnB.pressedFor(800);
    _held[2]    = M5.BtnC.pressedFor(800);
}

bool InputManager::wasPressed(VButton btn) const {
    if (btn == VButton::NONE) return false;
    return _pressed[static_cast<uint8_t>(btn)];
}

bool InputManager::wasHeld(VButton btn) const {
    if (btn == VButton::NONE) return false;
    return _held[static_cast<uint8_t>(btn)];
}

bool InputManager::anyPressed() const {
    return _pressed[0] || _pressed[1] || _pressed[2];
}
