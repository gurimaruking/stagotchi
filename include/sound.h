#pragma once
#include <M5Unified.h>

enum class SoundEffect : uint8_t {
    BUTTON_PRESS,
    FEED,
    HAPPY,
    SAD,
    ATTENTION,
    GAME_WIN,
    GAME_LOSE,
    DISCIPLINE,
    EVOLUTION,
    HATCH,
    DEATH,
    MEDICINE,
};

class SoundManager {
public:
    void init();
    void play(SoundEffect fx);
    void setMute(bool m) { _muted = m; }
    bool isMuted() const { return _muted; }

private:
    bool _muted = false;
    void tone(uint16_t freq, uint16_t durationMs);
};
