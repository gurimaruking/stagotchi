#include "display.h"
#include "config.h"
#include "sprites.h"

// ======== Double-buffered rendering via M5Canvas (8-bit color = 76.8KB) ========
// All drawing goes to _canvas, then flush() pushes to screen atomically.

void DisplayManager::init() {
    M5.Display.setRotation(1);
    M5.Display.setBrightness(200);
    _canvas.setColorDepth(8);  // 8-bit = 76,800 bytes (fits in RAM)
    _canvas.createSprite(SCREEN_W, SCREEN_H);
    _canvas.fillSprite(TFT_BLACK);
    flush();
    Serial.println("[DISPLAY] init done (8bit canvas)");
}

void DisplayManager::flush() {
    _canvas.pushSprite(0, 0);
}

// Font helpers using M5GFX built-in Japanese fonts
void DisplayManager::setFontSmall() {
    _canvas.setFont(&fonts::lgfxJapanGothic_12);
}
void DisplayManager::setFontMedium() {
    _canvas.setFont(&fonts::lgfxJapanGothic_16);
}
void DisplayManager::setFontLarge() {
    _canvas.setFont(&fonts::lgfxJapanGothic_24);
}

// 1-bit sprite: draw fg AND bg pixels (no transparency flicker)
void DisplayManager::drawSprite1bit(int x, int y, int w, int h,
                                     const uint8_t* data, uint16_t fgColor,
                                     uint16_t bgColor) {
    int bytesPerRow = (w + 7) / 8;
    for (int row = 0; row < h; row++) {
        for (int col = 0; col < w; col++) {
            int byteIdx = row * bytesPerRow + col / 8;
            int bitIdx = 7 - (col % 8);
            uint8_t b = pgm_read_byte(&data[byteIdx]);
            if (b & (1 << bitIdx)) {
                _canvas.drawPixel(x + col, y + row, fgColor);
            } else {
                _canvas.drawPixel(x + col, y + row, bgColor);
            }
        }
    }
}

void DisplayManager::drawHearts(int x, int y, uint8_t filled, uint8_t max,
                                 uint16_t color) {
    for (uint8_t i = 0; i < max; i++) {
        const uint8_t* spr = (i < filled) ? SPR_HEART_FULL : SPR_HEART_EMPTY;
        uint16_t c = (i < filled) ? color : COL_HEART_E;
        drawSprite1bit(x + i * 12, y, 8, 8, spr, c, COL_WHITE);
    }
}

void DisplayManager::drawMenuIcons(uint8_t cursor) {
    _canvas.fillRect(0, 0, SCREEN_W, 32, COL_ICON_BG);
    // UTF-8 menu labels: 食 灯 遊 薬 掃 状 躾
    const char* labelsU[] = {"\xe9\xa3\x9f", "\xe7\x81\xaf", "\xe9\x81\x8a", "\xe8\x96\xac", "\xe6\x8e\x83", "\xe7\x8a\xb6", "\xe8\xba\xbe"};

    setFontSmall();
    for (int i = 0; i < ICON_COUNT; i++) {
        int ix = ICON_START_X + i * ICON_STEP;
        bool selected = (i == cursor);

        if (selected) {
            _canvas.fillRect(ix - 2, ICON_ROW_Y - 2, ICON_SIZE + 4, ICON_SIZE + 4, COL_ICON_SEL);
            _canvas.setTextColor(COL_WHITE, COL_ICON_SEL);
        } else {
            _canvas.fillRect(ix, ICON_ROW_Y, ICON_SIZE, ICON_SIZE, COL_WHITE);
            _canvas.setTextColor(COL_BLACK, COL_WHITE);
        }
        _canvas.setTextDatum(MC_DATUM);
        _canvas.drawString(labelsU[i], ix + ICON_SIZE / 2, ICON_ROW_Y + ICON_SIZE / 2);
    }
}

void DisplayManager::drawStatusBar(const PetData& pet) {
    _canvas.fillRect(0, STATUS_BAR_Y, SCREEN_W, STATUS_BAR_H, COL_STATUS_BG);
    _canvas.setTextColor(COL_STATUS_FG, COL_STATUS_BG);
    setFontSmall();
    _canvas.setTextDatum(ML_DATUM);

    char buf[40];
    snprintf(buf, sizeof(buf), "\xe5\xb9\xb4\xe9\xbd\xa2:%d\xe6\x99\x82  \xe4\xbd\x93\xe9\x87\x8d:%dg", pet.age, pet.weight);
    _canvas.drawString(buf, 8, STATUS_BAR_Y + STATUS_BAR_H / 2);

    if (pet.pendingAttention != AttentionType::NONE) {
        _canvas.setTextColor(COL_HEART, COL_STATUS_BG);
        _canvas.drawString("(!)", 220, STATUS_BAR_Y + STATUS_BAR_H / 2);
    }

    int batt = M5.Power.getBatteryLevel();
    if (batt >= 0) {
        _canvas.setTextColor(COL_STATUS_FG, COL_STATUS_BG);
        _canvas.setTextDatum(MR_DATUM);
        snprintf(buf, sizeof(buf), "%d%%", batt);
        _canvas.drawString(buf, SCREEN_W - 8, STATUS_BAR_Y + STATUS_BAR_H / 2);
    }
}

void DisplayManager::drawPetSprite(int cx, int cy, CharacterID charId, uint16_t bgColor) {
    const uint8_t* spr = getSpriteForCharacter(charId);
    drawSprite1bit(cx - SPRITE_W / 2, cy - SPRITE_H / 2, SPRITE_W, SPRITE_H,
                   spr, COL_BLACK, bgColor);
}

void DisplayManager::drawPoops(uint8_t count) {
    for (uint8_t i = 0; i < count; i++) {
        int px = PET_AREA_X + PET_AREA_W - 20 - i * 18;
        int py = PET_AREA_Y + PET_AREA_H - 20;
        drawSprite1bit(px, py, 12, 12, SPR_POOP, COL_POOP, COL_PET_BG);
    }
}

void DisplayManager::drawAttention(AttentionType type) {
    unsigned long now = millis();
    if (now - _lastBlinkMs > 500) {
        _blinkState = !_blinkState;
        _lastBlinkMs = now;
    }
    if (!_blinkState) return;

    int ax = PET_AREA_X + PET_AREA_W - 14;
    int ay = PET_AREA_Y + 4;
    drawSprite1bit(ax, ay, 8, 16, SPR_ATTENTION, COL_HEART, COL_PET_BG);

    if (type == AttentionType::SICK) {
        drawSprite1bit(ax - 16, ay, 12, 12, SPR_SKULL, COL_SICK, COL_PET_BG);
    }
}

// ========== Full Screen Drawing (all draw to _canvas, call flush() after) ==========

void DisplayManager::drawTitleScreen() {
    _canvas.fillSprite(TFT_BLACK);

    _canvas.setTextColor(TFT_WHITE, TFT_BLACK);
    _canvas.setTextDatum(MC_DATUM);

    setFontLarge();
    _canvas.drawString("\xe3\x81\x99\xe3\x81\x9f\xe3\x81\x94\xe3\x81\xa3\xe3\x81\xa1", SCREEN_W / 2, 55);
    // "すたごっち"

    setFontMedium();
    _canvas.drawString("- STAGOTCHI -", SCREEN_W / 2, 90);

    setFontSmall();
    _canvas.drawString("\xe3\x82\xb9\xe3\x82\xbf\xe3\x83\x83\xe3\x82\xaf\xe3\x83\x81\xe3\x83\xa3\xe3\x83\xb3\xe3\x82\x92\xe8\x82\xb2\xe3\x81\xa6\xe3\x82\x88\xe3\x81\x86\xef\xbc\x81", SCREEN_W / 2, 120);
    // "ｽﾀｯｸﾁｬﾝを育てよう！"

    drawPetSprite(SCREEN_W / 2, 170, CharacterID::EGG, TFT_BLACK);

    _canvas.setTextColor(0x7BCF, TFT_BLACK);
    setFontSmall();
    _canvas.drawString("\xe3\x83\x9c\xe3\x82\xbf\xe3\x83\xb3\xe3\x82\x92\xe6\x8a\xbc\xe3\x81\x97\xe3\x81\xa6\xe3\x81\xad", SCREEN_W / 2, 220);
    // "ボタンを押してね"
    flush();
}

void DisplayManager::drawNewOrContinue(uint8_t selection) {
    _canvas.fillSprite(COL_BG);

    _canvas.setTextColor(COL_BLACK, COL_BG);
    _canvas.setTextDatum(MC_DATUM);
    setFontLarge();
    _canvas.drawString("\xe3\x81\x99\xe3\x81\x9f\xe3\x81\x94\xe3\x81\xa3\xe3\x81\xa1", SCREEN_W / 2, 40);

    setFontMedium();
    // "はじめから" / "つづきから"
    const char* items[] = {
        "\xe3\x81\xaf\xe3\x81\x98\xe3\x82\x81\xe3\x81\x8b\xe3\x82\x89",
        "\xe3\x81\xa4\xe3\x81\xa5\xe3\x81\x8d\xe3\x81\x8b\xe3\x82\x89"
    };
    for (int i = 0; i < 2; i++) {
        if (i == selection) {
            _canvas.fillRect(60, 90 + i * 55, 200, 40, COL_ICON_SEL);
            _canvas.setTextColor(COL_WHITE, COL_ICON_SEL);
        } else {
            _canvas.fillRect(60, 90 + i * 55, 200, 40, COL_BG);
            _canvas.setTextColor(COL_BLACK, COL_BG);
        }
        _canvas.drawString(items[i], SCREEN_W / 2, 110 + i * 55);
    }

    setFontSmall();
    _canvas.setTextColor(COL_DARK, COL_BG);
    // "A:上  B:決定  C:下"
    _canvas.drawString("A:\xe4\xb8\x8a  B:\xe6\xb1\xba\xe5\xae\x9a  C:\xe4\xb8\x8b", SCREEN_W / 2, 220);
    flush();
}

void DisplayManager::drawEggHatching(float progress) {
    _canvas.fillSprite(COL_BG);
    _canvas.fillRect(PET_AREA_X, PET_AREA_Y, PET_AREA_W, PET_AREA_H, COL_PET_BG);

    int wobble = (int)(sin(progress * 20.0f) * 4.0f * progress);
    drawPetSprite(SCREEN_W / 2 + wobble, PET_AREA_Y + PET_AREA_H / 2,
                  CharacterID::EGG, COL_PET_BG);

    int barW = (int)(PET_AREA_W * 0.8f);
    int barX = PET_AREA_X + (PET_AREA_W - barW) / 2;
    int barY = PET_AREA_Y + PET_AREA_H + 8;
    _canvas.drawRect(barX, barY, barW, 10, COL_BLACK);
    int fillW = (int)(barW * progress) - 2;
    if (fillW > 0) {
        _canvas.fillRect(barX + 1, barY + 1, fillW, 8, COL_ICON_SEL);
    }

    _canvas.setTextColor(COL_BLACK, COL_BG);
    setFontSmall();
    _canvas.setTextDatum(MC_DATUM);
    // "たまごがかえるよ..."
    _canvas.drawString("\xe3\x81\x9f\xe3\x81\xbe\xe3\x81\x94\xe3\x81\x8c\xe3\x81\x8b\xe3\x81\x88\xe3\x82\x8b\xe3\x82\x88...", SCREEN_W / 2, barY + 24);
    flush();
}

void DisplayManager::drawGameplay(const PetData& pet, const CharacterDef& charDef,
                                   uint8_t menuCursor) {
    _canvas.fillSprite(COL_BG);

    drawMenuIcons(menuCursor);

    _canvas.fillRect(PET_AREA_X, PET_AREA_Y, PET_AREA_W, PET_AREA_H, COL_PET_BG);
    _canvas.drawRect(PET_AREA_X - 1, PET_AREA_Y - 1, PET_AREA_W + 2, PET_AREA_H + 2, COL_DARK);

    drawPetSprite(PET_AREA_X + PET_AREA_W / 2, PET_AREA_Y + PET_AREA_H / 2,
                  pet.characterId, COL_PET_BG);

    _canvas.setTextColor(COL_BLACK, COL_PET_BG);
    setFontSmall();
    _canvas.setTextDatum(MC_DATUM);
    _canvas.drawString(charDef.nameJP, PET_AREA_X + PET_AREA_W / 2,
                       PET_AREA_Y + PET_AREA_H - 14);

    drawPoops(pet.poopCount);

    if (pet.isSick) {
        drawSprite1bit(PET_AREA_X + 4, PET_AREA_Y + 4, 12, 12, SPR_SKULL, COL_SICK, COL_PET_BG);
    }

    if (pet.pendingAttention != AttentionType::NONE) {
        drawAttention(pet.pendingAttention);
    }

    drawStatusBar(pet);
    flush();
}

void DisplayManager::drawGameplayNoFlush(const PetData& pet, const CharacterDef& charDef,
                                          uint8_t menuCursor) {
    _canvas.fillSprite(COL_BG);
    drawMenuIcons(menuCursor);
    _canvas.fillRect(PET_AREA_X, PET_AREA_Y, PET_AREA_W, PET_AREA_H, COL_PET_BG);
    _canvas.drawRect(PET_AREA_X - 1, PET_AREA_Y - 1, PET_AREA_W + 2, PET_AREA_H + 2, COL_DARK);
    drawPetSprite(PET_AREA_X + PET_AREA_W / 2, PET_AREA_Y + PET_AREA_H / 2,
                  pet.characterId, COL_PET_BG);
    _canvas.setTextColor(COL_BLACK, COL_PET_BG);
    setFontSmall();
    _canvas.setTextDatum(MC_DATUM);
    _canvas.drawString(charDef.nameJP, PET_AREA_X + PET_AREA_W / 2,
                       PET_AREA_Y + PET_AREA_H - 14);
    drawPoops(pet.poopCount);
    if (pet.isSick) {
        drawSprite1bit(PET_AREA_X + 4, PET_AREA_Y + 4, 12, 12, SPR_SKULL, COL_SICK, COL_PET_BG);
    }
    if (pet.pendingAttention != AttentionType::NONE) {
        drawAttention(pet.pendingAttention);
    }
    drawStatusBar(pet);
    // No flush - caller will overlay feed menu then flush
}

void DisplayManager::drawFeedMenu(uint8_t subCursor) {
    // NOTE: This draws onto _canvas WITHOUT flushing.
    // Caller must call flush() after drawGameplay + drawFeedMenu.
    int mx = SCREEN_W / 2 - 70;
    int my = 80;
    _canvas.fillRect(mx, my, 140, 90, COL_WHITE);
    _canvas.drawRect(mx, my, 140, 90, COL_BLACK);

    setFontMedium();
    _canvas.setTextDatum(ML_DATUM);
    // "ごはん" / "おやつ" / "やめる"
    const char* items[] = {
        "\xe3\x81\x94\xe3\x81\xaf\xe3\x82\x93",
        "\xe3\x81\x8a\xe3\x82\x84\xe3\x81\xa4",
        "\xe3\x82\x84\xe3\x82\x81\xe3\x82\x8b"
    };
    for (int i = 0; i < 3; i++) {
        uint16_t fg = (i == subCursor) ? COL_ICON_SEL : COL_BLACK;
        _canvas.setTextColor(fg, COL_WHITE);
        char buf[30];
        snprintf(buf, sizeof(buf), "%s%s", (i == subCursor) ? "\xe2\x96\xb6 " : "  ", items[i]);
        _canvas.drawString(buf, mx + 10, my + 16 + i * 24);
    }
    // No flush here - caller handles it
}

void DisplayManager::drawStatScreen(const PetData& pet, const CharacterDef& charDef) {
    _canvas.fillSprite(COL_BG);
    _canvas.fillRect(20, 15, 280, 210, COL_WHITE);
    _canvas.drawRect(20, 15, 280, 210, COL_BLACK);

    _canvas.setTextColor(COL_BLACK, COL_WHITE);
    setFontMedium();
    _canvas.setTextDatum(ML_DATUM);

    int x = 40, y = 38;
    char buf[50];

    // "なまえ: XXX"
    snprintf(buf, sizeof(buf), "\xe3\x81\xaa\xe3\x81\xbe\xe3\x81\x88: %s", charDef.nameJP);
    _canvas.drawString(buf, x, y); y += 28;

    // "ねんれい: X じかん"
    snprintf(buf, sizeof(buf), "\xe3\x81\xad\xe3\x82\x93\xe3\x82\x8c\xe3\x81\x84: %d \xe3\x81\x98\xe3\x81\x8b\xe3\x82\x93", pet.age);
    _canvas.drawString(buf, x, y); y += 28;

    // "たいじゅう: Xg"
    snprintf(buf, sizeof(buf), "\xe3\x81\x9f\xe3\x81\x84\xe3\x81\x98\xe3\x82\x85\xe3\x81\x86: %dg", pet.weight);
    _canvas.drawString(buf, x, y); y += 28;

    // "おなか:"
    _canvas.drawString("\xe3\x81\x8a\xe3\x81\xaa\xe3\x81\x8b:", x, y);
    drawHearts(x + 80, y - 4, pet.hunger, MAX_HUNGER, COL_HEART);
    y += 28;

    // "ごきげん:"
    _canvas.drawString("\xe3\x81\x94\xe3\x81\x8d\xe3\x81\x92\xe3\x82\x93:", x, y);
    drawHearts(x + 80, y - 4, pet.happiness, MAX_HAPPY, COL_HEART);
    y += 28;

    // "しつけ: XX%"
    snprintf(buf, sizeof(buf), "\xe3\x81\x97\xe3\x81\xa4\xe3\x81\x91: %d%%", pet.discipline);
    _canvas.drawString(buf, x, y);

    _canvas.setTextDatum(MC_DATUM);
    _canvas.setTextColor(COL_DARK, COL_BG);
    setFontSmall();
    // "ボタンでもどる"
    _canvas.drawString("\xe3\x83\x9c\xe3\x82\xbf\xe3\x83\xb3\xe3\x81\xa7\xe3\x82\x82\xe3\x81\xa9\xe3\x82\x8b", SCREEN_W / 2, 232);
    flush();
}

void DisplayManager::drawEvolution(const char* fromName, const char* toName, float progress) {
    _canvas.fillSprite(COL_BG);
    _canvas.fillRect(PET_AREA_X, PET_AREA_Y, PET_AREA_W, PET_AREA_H, COL_WHITE);

    _canvas.setTextColor(COL_BLACK, COL_WHITE);
    _canvas.setTextDatum(MC_DATUM);

    if (progress < 0.5f) {
        setFontMedium();
        _canvas.drawString(fromName, SCREEN_W / 2, PET_AREA_Y + 40);
        setFontLarge();
        _canvas.drawString("...", SCREEN_W / 2, PET_AREA_Y + PET_AREA_H / 2);
    } else {
        setFontSmall();
        _canvas.drawString("!", SCREEN_W / 2, PET_AREA_Y + 30);
        setFontLarge();
        _canvas.drawString(toName, SCREEN_W / 2, PET_AREA_Y + PET_AREA_H / 2);
    }

    setFontMedium();
    _canvas.setTextColor(COL_BLACK, COL_BG);
    // "しんかした！"
    _canvas.drawString("\xe3\x81\x97\xe3\x82\x93\xe3\x81\x8b\xe3\x81\x97\xe3\x81\x9f\xef\xbc\x81", SCREEN_W / 2, PET_AREA_Y + PET_AREA_H + 12);
    flush();
}

void DisplayManager::drawSleepScreen(const PetData& pet, const CharacterDef& charDef,
                                      bool lightOff) {
    if (lightOff) {
        _canvas.fillSprite(TFT_BLACK);
        _canvas.setTextColor(0x4208, TFT_BLACK);
        setFontLarge();
        _canvas.setTextDatum(MC_DATUM);
        _canvas.drawString("Z z z . . .", SCREEN_W / 2, SCREEN_H / 2);
    } else {
        _canvas.fillSprite(COL_BG);
        _canvas.fillRect(PET_AREA_X, PET_AREA_Y, PET_AREA_W, PET_AREA_H, COL_PET_BG);
        drawPetSprite(PET_AREA_X + PET_AREA_W / 2, PET_AREA_Y + PET_AREA_H / 2,
                      pet.characterId, COL_PET_BG);
        drawSprite1bit(PET_AREA_X + PET_AREA_W / 2 + 30,
                       PET_AREA_Y + PET_AREA_H / 2 - 30,
                       8, 8, SPR_ZZZ, COL_DARK, COL_PET_BG);
        _canvas.setTextColor(COL_BLACK, COL_BG);
        setFontSmall();
        _canvas.setTextDatum(MC_DATUM);
        // "おやすみ中...ライトを消してね"
        _canvas.drawString("\xe3\x81\x8a\xe3\x82\x84\xe3\x81\x99\xe3\x81\xbf\xe4\xb8\xad...\xe3\x83\xa9\xe3\x82\xa4\xe3\x83\x88\xe3\x82\x92\xe6\xb6\x88\xe3\x81\x97\xe3\x81\xa6\xe3\x81\xad", SCREEN_W / 2,
                          PET_AREA_Y + PET_AREA_H + 10);
        drawMenuIcons(1);
    }
    drawStatusBar(pet);
    flush();
}

void DisplayManager::drawDeathScreen(uint8_t cause) {
    _canvas.fillSprite(TFT_BLACK);
    drawPetSprite(SCREEN_W / 2, 80, CharacterID::GHOST, TFT_BLACK);

    _canvas.setTextColor(TFT_WHITE, TFT_BLACK);
    _canvas.setTextDatum(MC_DATUM);
    setFontMedium();

    // "ほうち...", "びょうき...", "じゅみょう..."
    const char* reasons[] = {
        "\xe3\x81\xbb\xe3\x81\x86\xe3\x81\xa1...",
        "\xe3\x81\xb3\xe3\x82\x87\xe3\x81\x86\xe3\x81\x8d...",
        "\xe3\x81\x98\xe3\x82\x85\xe3\x81\xbf\xe3\x82\x87\xe3\x81\x86..."
    };
    uint8_t idx = (cause < 3) ? cause : 0;
    _canvas.drawString(reasons[idx], SCREEN_W / 2, 140);

    setFontLarge();
    // "さようなら..."
    _canvas.drawString("\xe3\x81\x95\xe3\x82\x88\xe3\x81\x86\xe3\x81\xaa\xe3\x82\x89...", SCREEN_W / 2, 175);

    setFontSmall();
    _canvas.setTextColor(0x7BCF, TFT_BLACK);
    // "ボタンを押してね"
    _canvas.drawString("\xe3\x83\x9c\xe3\x82\xbf\xe3\x83\xb3\xe3\x82\x92\xe6\x8a\xbc\xe3\x81\x97\xe3\x81\xa6\xe3\x81\xad", SCREEN_W / 2, 220);
    flush();
}

void DisplayManager::drawMinigame(uint8_t round, uint8_t currentNum, uint8_t wins,
                                   uint8_t lastResult, bool showResult) {
    _canvas.fillSprite(COL_BG);
    _canvas.fillRect(40, 30, 240, 170, COL_WHITE);
    _canvas.drawRect(40, 30, 240, 170, COL_BLACK);

    _canvas.setTextColor(COL_BLACK, COL_WHITE);
    _canvas.setTextDatum(MC_DATUM);
    setFontSmall();

    char buf[40];
    // "だい X かい  かち: Y"
    snprintf(buf, sizeof(buf), "\xe3\x81\xa0\xe3\x81\x84 %d \xe3\x81\x8b\xe3\x81\x84  \xe3\x81\x8b\xe3\x81\xa1: %d", round, wins);
    _canvas.drawString(buf, SCREEN_W / 2, 50);

    setFontLarge();
    _canvas.setFont(&fonts::lgfxJapanGothic_36);
    snprintf(buf, sizeof(buf), "%d", currentNum);
    _canvas.drawString(buf, SCREEN_W / 2, 110);

    if (showResult) {
        setFontMedium();
        if (lastResult == 1) {
            _canvas.setTextColor(0x07E0, COL_WHITE);
            // "あたり！"
            _canvas.drawString("\xe3\x81\x82\xe3\x81\x9f\xe3\x82\x8a\xef\xbc\x81", SCREEN_W / 2, 160);
        } else {
            _canvas.setTextColor(COL_HEART, COL_WHITE);
            // "はずれ！"
            _canvas.drawString("\xe3\x81\xaf\xe3\x81\x9a\xe3\x82\x8c\xef\xbc\x81", SCREEN_W / 2, 160);
        }
    } else {
        setFontSmall();
        // "おおきい？ちいさい？"
        _canvas.drawString("\xe3\x81\x8a\xe3\x81\x8a\xe3\x81\x8d\xe3\x81\x84\xef\xbc\x9f\xe3\x81\xa1\xe3\x81\x84\xe3\x81\x95\xe3\x81\x84\xef\xbc\x9f", SCREEN_W / 2, 150);
        // "A:大  C:小  B:やめる"
        _canvas.drawString("A:\xe5\xa4\xa7  C:\xe5\xb0\x8f  B:\xe3\x82\x84\xe3\x82\x81\xe3\x82\x8b", SCREEN_W / 2, 175);
    }
    flush();
}
