#pragma once
#include <M5Unified.h>
#include <M5GFX.h>
#include "character.h"
#include "pet.h"
#include "menu.h"

class DisplayManager {
public:
    void init();
    void flush();  // push canvas to screen

    void drawTitleScreen();
    void drawNewOrContinue(uint8_t selection);
    void drawEggHatching(float progress);
    void drawGameplay(const PetData& pet, const CharacterDef& charDef, uint8_t menuCursor);
    void drawGameplayNoFlush(const PetData& pet, const CharacterDef& charDef, uint8_t menuCursor);
    void drawFeedMenu(uint8_t subCursor);
    void drawStatScreen(const PetData& pet, const CharacterDef& charDef);
    void drawEvolution(const char* fromName, const char* toName, float progress);
    void drawSleepScreen(const PetData& pet, const CharacterDef& charDef, bool lightOff);
    void drawDeathScreen(uint8_t cause);
    void drawMinigame(uint8_t round, uint8_t currentNum, uint8_t wins,
                      uint8_t lastResult, bool showResult);

private:
    M5Canvas _canvas{&M5.Display};
    bool _blinkState = false;
    unsigned long _lastBlinkMs = 0;

    void drawSprite1bit(int x, int y, int w, int h, const uint8_t* data,
                        uint16_t fgColor, uint16_t bgColor);
    void drawMenuIcons(uint8_t cursor);
    void drawStatusBar(const PetData& pet);
    void drawHearts(int x, int y, uint8_t filled, uint8_t max, uint16_t color);
    void drawPetSprite(int cx, int cy, CharacterID charId, uint16_t bgColor);
    void drawPoops(uint8_t count);
    void drawAttention(AttentionType type);

    // Font helpers
    void setFontSmall();   // ~12px Japanese
    void setFontMedium();  // ~16px Japanese
    void setFontLarge();   // ~24px Japanese
};
