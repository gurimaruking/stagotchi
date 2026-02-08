#pragma once
#include <M5Unified.h>

enum class VButton : uint8_t {
    LEFT = 0,    // BtnA
    CENTER,      // BtnB
    RIGHT,       // BtnC
    NONE
};

class InputManager {
public:
    void init();
    void update();

    bool wasPressed(VButton btn) const;
    bool wasHeld(VButton btn) const;
    bool anyPressed() const;

private:
    bool _pressed[3] = {};
    bool _held[3]    = {};
};
