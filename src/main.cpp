// =============================================
//  STAGOTCHI (すたごっち)
//  Stack-chan Virtual Pet for M5Stack
//  Raise your own Stack-chan derivative!
// =============================================

#include <M5Unified.h>
#include "config.h"
#include "game_state.h"
#include "character.h"
#include "pet.h"
#include "display.h"
#include "input.h"
#include "menu.h"
#include "minigame.h"
#include "sound.h"

// ===== Global Managers =====
StateMachine   gState;
PetManager     gPet;
DisplayManager gDisplay;
InputManager   gInput;
MenuSystem     gMenu;
MiniGame       gGame;
SoundManager   gSound;

// ===== Timers =====
unsigned long gLastSaveMs     = 0;
unsigned long gLastDrawMs     = 0;
constexpr unsigned long DRAW_INTERVAL_MS = 200; // Redraw every 200ms (5fps) to prevent flicker
unsigned long gEvoAnimStartMs = 0;
unsigned long gEvoAnimDuration= 3000;
CharacterID   gEvoFromChar    = CharacterID::NONE;
uint8_t       gNewContinueSel = 0;  // 0=New, 1=Continue
bool          gForceRedraw    = true;

// ===== Helper =====
uint8_t getCurrentHour() {
    auto dt = M5.Rtc.getDateTime();
    if (dt.date.year > 2020) {
        return dt.time.hours;
    }
    // Fallback: assume noon start
    return (12 + (millis() / 3600000UL)) % 24;
}

// ===== State Handlers =====

void handleTitleScreen() {
    if (gInput.anyPressed()) {
        gSound.play(SoundEffect::BUTTON_PRESS);
        gForceRedraw = true;
        if (gState.hasSaveData()) {
            gNewContinueSel = 1;  // default to Continue
            gState.transition(GameState::NEW_OR_CONTINUE);
            gDisplay.drawNewOrContinue(gNewContinueSel);
        } else {
            // New game directly
            gPet.initNewEgg(millis());
            gState.transition(GameState::EGG_HATCHING);
        }
    }
}

void handleNewOrContinue() {
    bool redraw = false;
    if (gInput.wasPressed(VButton::LEFT)) {
        gNewContinueSel = (gNewContinueSel == 0) ? 1 : 0;
        gSound.play(SoundEffect::BUTTON_PRESS);
        redraw = true;
    }
    if (gInput.wasPressed(VButton::RIGHT)) {
        gNewContinueSel = (gNewContinueSel == 0) ? 1 : 0;
        gSound.play(SoundEffect::BUTTON_PRESS);
        redraw = true;
    }
    if (gInput.wasPressed(VButton::CENTER)) {
        gSound.play(SoundEffect::BUTTON_PRESS);
        if (gNewContinueSel == 0) {
            // New Game
            gState.clearSave();
            gPet.initNewEgg(millis());
            gState.transition(GameState::EGG_HATCHING);
        } else {
            // Continue
            PetData loaded;
            if (gState.loadGame(loaded)) {
                gPet.loadFromSave(loaded);
                if (gPet.data().isDead) {
                    gState.transition(GameState::DEATH_SCREEN);
                    gDisplay.drawDeathScreen(gPet.data().deathCause);
                } else if (gPet.data().stage == LifeStage::EGG) {
                    gState.transition(GameState::EGG_HATCHING);
                } else if (gPet.data().isAsleep) {
                    gState.transition(GameState::SLEEPING);
                } else {
                    gState.transition(GameState::GAMEPLAY);
                }
            } else {
                // Load failed, start new
                gPet.initNewEgg(millis());
                gState.transition(GameState::EGG_HATCHING);
            }
        }
        return;
    }
    if (redraw) {
        gDisplay.drawNewOrContinue(gNewContinueSel);
    }
}

void handleEggHatching(unsigned long now) {
    gPet.update(now, getCurrentHour());

    if (gPet.isEvolving()) {
        gSound.play(SoundEffect::HATCH);
        gEvoFromChar = gPet.data().characterId;
        gPet.doEvolve(now);
        gEvoAnimStartMs = now;
        gForceRedraw = true;
        gState.transition(GameState::EVOLUTION);
        return;
    }

    if (now - gLastDrawMs >= DRAW_INTERVAL_MS || gForceRedraw) {
        float progress = (float)(now - gPet.data().stageStartMs) / (float)EGG_HATCH_MS;
        if (progress > 1.0f) progress = 1.0f;
        gDisplay.drawEggHatching(progress);
        gLastDrawMs = now;
        gForceRedraw = false;
    }
}

void handleGameplay(unsigned long now, uint8_t hour) {
    // Update pet
    gPet.update(now, hour);

    // Check death
    if (gPet.data().isDead) {
        gSound.play(SoundEffect::DEATH);
        gState.transition(GameState::DEATH_SCREEN);
        gDisplay.drawDeathScreen(gPet.data().deathCause);
        gState.saveGame(gPet.data());
        return;
    }

    // Check evolution
    if (gPet.isEvolving()) {
        gSound.play(SoundEffect::EVOLUTION);
        gEvoFromChar = gPet.data().characterId;
        gPet.doEvolve(now);
        gEvoAnimStartMs = now;
        gState.transition(GameState::EVOLUTION);
        return;
    }

    // Check sleep
    if (gPet.data().isAsleep) {
        gState.transition(GameState::SLEEPING);
        return;
    }

    // Input handling
    if (gInput.wasPressed(VButton::LEFT)) {
        gMenu.moveCursorLeft();
        gSound.play(SoundEffect::BUTTON_PRESS);
        gForceRedraw = true;
    }
    if (gInput.wasPressed(VButton::RIGHT)) {
        gMenu.moveCursorRight();
        gSound.play(SoundEffect::BUTTON_PRESS);
        gForceRedraw = true;
    }
    if (gInput.wasPressed(VButton::CENTER)) {
        gSound.play(SoundEffect::BUTTON_PRESS);
        gForceRedraw = true;
        MenuItem item = gMenu.getCurrentItem();
        switch (item) {
            case MenuItem::FEED:
                gMenu.open();
                gState.transition(GameState::MENU_FEED);
                break;
            case MenuItem::LIGHT:
                if (gPet.toggleLight()) {
                    gSound.play(SoundEffect::HAPPY);
                }
                break;
            case MenuItem::PLAY:
                if (gPet.startGame()) {
                    gGame.start();
                    gState.transition(GameState::MINIGAME);
                } else {
                    gSound.play(SoundEffect::SAD);
                }
                break;
            case MenuItem::MEDICINE:
                if (gPet.giveMedicine()) {
                    gSound.play(SoundEffect::MEDICINE);
                } else {
                    gSound.play(SoundEffect::SAD);
                }
                break;
            case MenuItem::CLEAN:
                if (gPet.clean()) {
                    gSound.play(SoundEffect::HAPPY);
                } else {
                    gSound.play(SoundEffect::SAD);
                }
                break;
            case MenuItem::STATUS:
                gState.transition(GameState::STAT_SCREEN);
                break;
            case MenuItem::DISCIPLINE:
                if (gPet.discipline()) {
                    gSound.play(SoundEffect::DISCIPLINE);
                } else {
                    gSound.play(SoundEffect::SAD);
                }
                break;
            default:
                break;
        }
    }

    // Draw (throttled to avoid flicker)
    if (now - gLastDrawMs >= DRAW_INTERVAL_MS || gForceRedraw) {
        const auto& charDef = getCharacterDef(gPet.data().characterId);
        gDisplay.drawGameplay(gPet.data(), charDef, gMenu.getCursor());
        gLastDrawMs = now;
        gForceRedraw = false;
    }
}

void handleFeedMenu(unsigned long now, uint8_t hour) {
    // Still update pet in background
    gPet.update(now, hour);

    if (gInput.wasPressed(VButton::LEFT)) {
        gMenu.feedSubUp();
        gSound.play(SoundEffect::BUTTON_PRESS);
    }
    if (gInput.wasPressed(VButton::RIGHT)) {
        gMenu.feedSubDown();
        gSound.play(SoundEffect::BUTTON_PRESS);
    }
    if (gInput.wasPressed(VButton::CENTER)) {
        gSound.play(SoundEffect::BUTTON_PRESS);
        FeedChoice choice = gMenu.getFeedChoice();
        switch (choice) {
            case FeedChoice::MEAL:
                if (gPet.feedMeal()) {
                    gSound.play(SoundEffect::FEED);
                } else {
                    gSound.play(SoundEffect::SAD);
                }
                gMenu.close();
                gState.transition(GameState::GAMEPLAY);
                break;
            case FeedChoice::SNACK:
                if (gPet.feedSnack()) {
                    gSound.play(SoundEffect::FEED);
                } else {
                    gSound.play(SoundEffect::SAD);
                }
                gMenu.close();
                gState.transition(GameState::GAMEPLAY);
                break;
            case FeedChoice::CANCEL:
                gMenu.close();
                gState.transition(GameState::GAMEPLAY);
                break;
            default:
                break;
        }
        return;
    }

    // Draw gameplay + feed overlay in single flush (no flicker)
    if (now - gLastDrawMs >= DRAW_INTERVAL_MS || gForceRedraw) {
        const auto& charDef = getCharacterDef(gPet.data().characterId);
        gDisplay.drawGameplayNoFlush(gPet.data(), charDef, gMenu.getCursor());
        gDisplay.drawFeedMenu(gMenu.getSubCursor());
        gDisplay.flush();
        gLastDrawMs = now;
        gForceRedraw = false;
    }
}

void handleMinigame(unsigned long now) {
    gGame.update(now);

    if (gGame.isFinished()) {
        if (gGame.isWin()) {
            gPet.onGameWin();
            gSound.play(SoundEffect::GAME_WIN);
        } else {
            gPet.onGameLose();
            gSound.play(SoundEffect::GAME_LOSE);
        }
        gState.transition(GameState::GAMEPLAY);
        return;
    }

    if (!gGame.showingResult()) {
        if (gInput.wasPressed(VButton::LEFT)) {
            gGame.guessHigher();
            gSound.play(SoundEffect::BUTTON_PRESS);
        }
        if (gInput.wasPressed(VButton::RIGHT)) {
            gGame.guessLower();
            gSound.play(SoundEffect::BUTTON_PRESS);
        }
        if (gInput.wasPressed(VButton::CENTER)) {
            // Quit game
            gPet.onGameLose();
            gState.transition(GameState::GAMEPLAY);
            return;
        }
    }

    if (now - gLastDrawMs >= DRAW_INTERVAL_MS || gForceRedraw) {
        gDisplay.drawMinigame(gGame.currentRound(), gGame.currentNumber(),
                               gGame.wins(), gGame.lastResult(),
                               gGame.showingResult());
        gLastDrawMs = now;
        gForceRedraw = false;
    }
}

void handleEvolution(unsigned long now) {
    float progress = (float)(now - gEvoAnimStartMs) / (float)gEvoAnimDuration;
    if (progress > 1.0f) progress = 1.0f;

    if (now - gLastDrawMs >= DRAW_INTERVAL_MS || gForceRedraw) {
        const auto& fromDef = getCharacterDef(gEvoFromChar);
        const auto& toDef = getCharacterDef(gPet.data().characterId);
        gDisplay.drawEvolution(fromDef.nameJP, toDef.nameJP, progress);
        gLastDrawMs = now;
        gForceRedraw = false;
    }

    if (progress >= 1.0f && gInput.anyPressed()) {
        gForceRedraw = true;
        gState.transition(GameState::GAMEPLAY);
    }
}

void handleSleeping(unsigned long now, uint8_t hour) {
    gPet.update(now, hour);

    if (!gPet.data().isAsleep) {
        gSound.play(SoundEffect::HAPPY);
        gForceRedraw = true;
        gState.transition(GameState::GAMEPLAY);
        return;
    }

    // Allow toggling light
    if (gInput.wasPressed(VButton::CENTER)) {
        gPet.toggleLight();
        gSound.play(SoundEffect::BUTTON_PRESS);
        gForceRedraw = true;
    }

    if (now - gLastDrawMs >= DRAW_INTERVAL_MS || gForceRedraw) {
        const auto& charDef = getCharacterDef(gPet.data().characterId);
        gDisplay.drawSleepScreen(gPet.data(), charDef, gPet.data().lightOff);
        gLastDrawMs = now;
        gForceRedraw = false;
    }
}

void handleStatScreen() {
    if (gInput.anyPressed()) {
        gSound.play(SoundEffect::BUTTON_PRESS);
        gForceRedraw = true;
        gState.transition(GameState::GAMEPLAY);
        return;
    }
    if (gForceRedraw) {
        const auto& charDef = getCharacterDef(gPet.data().characterId);
        gDisplay.drawStatScreen(gPet.data(), charDef);
        gForceRedraw = false;
    }
}

void handleDeathScreen() {
    if (gInput.anyPressed()) {
        gSound.play(SoundEffect::BUTTON_PRESS);
        gState.clearSave();
        gState.transition(GameState::TITLE_SCREEN);
        gDisplay.drawTitleScreen();
    }
}

// ===== Arduino Entry Points =====

void setup() {
    auto cfg = M5.config();
    M5.begin(cfg);

    randomSeed(analogRead(0) ^ millis());

    gDisplay.init();
    gInput.init();
    gSound.init();
    gMenu.init();
    gState.init();

    gState.transition(GameState::TITLE_SCREEN);
    gDisplay.drawTitleScreen();
}

void loop() {
    M5.update();
    gInput.update();

    unsigned long now = millis();
    uint8_t hour = getCurrentHour();

    switch (gState.current()) {
        case GameState::TITLE_SCREEN:
            handleTitleScreen();
            break;
        case GameState::NEW_OR_CONTINUE:
            handleNewOrContinue();
            break;
        case GameState::EGG_HATCHING:
            handleEggHatching(now);
            break;
        case GameState::GAMEPLAY:
            handleGameplay(now, hour);
            break;
        case GameState::MENU_FEED:
            handleFeedMenu(now, hour);
            break;
        case GameState::MINIGAME:
            handleMinigame(now);
            break;
        case GameState::EVOLUTION:
            handleEvolution(now);
            break;
        case GameState::SLEEPING:
            handleSleeping(now, hour);
            break;
        case GameState::STAT_SCREEN:
            handleStatScreen();
            break;
        case GameState::DEATH_SCREEN:
            handleDeathScreen();
            break;
    }

    // Autosave during active gameplay
    if (gState.current() == GameState::GAMEPLAY ||
        gState.current() == GameState::SLEEPING ||
        gState.current() == GameState::MENU_FEED) {
        if (now - gLastSaveMs > AUTOSAVE_INTERVAL_MS) {
            gState.saveGame(gPet.data());
            gLastSaveMs = now;
        }
    }

    delay(16);  // ~60fps
}
