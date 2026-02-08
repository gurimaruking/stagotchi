#pragma once
#include "pet.h"
#include <Preferences.h>

enum class GameState : uint8_t {
    TITLE_SCREEN,
    NEW_OR_CONTINUE,
    EGG_HATCHING,
    GAMEPLAY,
    MENU_FEED,
    MINIGAME,
    EVOLUTION,
    SLEEPING,
    STAT_SCREEN,
    DEATH_SCREEN,
};

class StateMachine {
public:
    void init();
    void transition(GameState newState);
    GameState current() const { return _current; }
    GameState previous() const { return _previous; }

    bool hasSaveData();
    void saveGame(const PetData& pet);
    bool loadGame(PetData& pet);
    void clearSave();

private:
    GameState _current  = GameState::TITLE_SCREEN;
    GameState _previous = GameState::TITLE_SCREEN;
    Preferences _prefs;
};
