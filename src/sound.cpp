#include "sound.h"

void SoundManager::init() {
    M5.Speaker.setVolume(64);
    // Don't start speaker yet - only enable when playing
    M5.Speaker.end();  // Keep amp OFF to prevent high-freq noise
}

void SoundManager::tone(uint16_t freq, uint16_t durationMs) {
    if (_muted) return;
    M5.Speaker.tone(freq, durationMs);
    delay(durationMs + 10);
    M5.Speaker.stop();
}

void SoundManager::play(SoundEffect fx) {
    if (_muted) return;

    // Enable speaker amp before playing
    M5.Speaker.begin();
    M5.Speaker.setVolume(64);

    switch (fx) {
        case SoundEffect::BUTTON_PRESS:
            tone(1000, 30);
            break;
        case SoundEffect::FEED:
            tone(800, 80);
            tone(1000, 80);
            break;
        case SoundEffect::HAPPY:
            tone(1200, 100);
            tone(1500, 100);
            tone(1800, 150);
            break;
        case SoundEffect::SAD:
            tone(400, 200);
            tone(300, 300);
            break;
        case SoundEffect::ATTENTION:
            tone(2000, 100);
            delay(100);
            tone(2000, 100);
            break;
        case SoundEffect::GAME_WIN:
            tone(1000, 100);
            tone(1200, 100);
            tone(1500, 100);
            tone(2000, 200);
            break;
        case SoundEffect::GAME_LOSE:
            tone(500, 150);
            tone(400, 150);
            tone(300, 200);
            break;
        case SoundEffect::DISCIPLINE:
            tone(600, 100);
            tone(800, 100);
            break;
        case SoundEffect::EVOLUTION:
            for (int f = 800; f < 2000; f += 100) {
                tone(f, 40);
            }
            tone(2000, 300);
            break;
        case SoundEffect::HATCH:
            tone(500, 100);
            tone(700, 100);
            tone(1000, 100);
            tone(1500, 200);
            break;
        case SoundEffect::DEATH:
            tone(600, 300);
            tone(500, 300);
            tone(400, 300);
            tone(300, 500);
            break;
        case SoundEffect::MEDICINE:
            tone(1000, 80);
            tone(1200, 80);
            break;
    }

    // Disable speaker amp after playing to prevent high-freq noise
    M5.Speaker.stop();
    M5.Speaker.end();
}
