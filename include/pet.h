#pragma once
#include "character.h"
#include <cstdint>

enum class AttentionType : uint8_t {
    NONE = 0,
    HUNGRY,
    UNHAPPY,
    DISCIPLINE,
    SICK,
    POOP,
    SLEEP,
};

struct PetData {
    CharacterID   characterId   = CharacterID::EGG;
    LifeStage     stage         = LifeStage::EGG;

    uint8_t  hunger        = 4;
    uint8_t  happiness     = 4;
    uint8_t  discipline    = 0;
    uint8_t  weight        = 5;
    uint16_t age           = 0;   // hours

    uint8_t  poopCount     = 0;
    bool     isSick        = false;
    uint8_t  sicknessLevel = 0;
    uint8_t  medicineGiven = 0;

    bool     isAsleep      = false;
    bool     lightOff      = false;

    uint8_t  careMistakes     = 0;  // per-stage
    uint8_t  disciplineCalls  = 0;
    uint16_t totalCareMistakes= 0;
    uint16_t totalAge         = 0;

    AttentionType pendingAttention = AttentionType::NONE;
    unsigned long attentionStartMs = 0;

    unsigned long lastHungerDecayMs = 0;
    unsigned long lastHappyDecayMs  = 0;
    unsigned long lastPoopMs        = 0;
    unsigned long lastAgeTickMs     = 0;
    unsigned long lastSickCheckMs   = 0;
    unsigned long lastDisciplineMs  = 0;
    unsigned long stageStartMs      = 0;

    bool     readyToEvolve = false;
    bool     isDead        = false;
    uint8_t  deathCause    = 0;  // 0=neglect, 1=sickness, 2=old age
};

class PetManager {
public:
    void initNewEgg(unsigned long nowMs);
    void loadFromSave(const PetData& d);
    PetData& data();
    const PetData& data() const;

    void update(unsigned long nowMs, uint8_t currentHour);

    // Player actions
    bool feedMeal();
    bool feedSnack();
    bool startGame();
    bool discipline();
    bool giveMedicine();
    bool clean();
    bool toggleLight();

    // Game results
    void onGameWin();
    void onGameLose();

    // Evolution
    bool isEvolving() const;
    void doEvolve(unsigned long nowMs);

    bool hasAttention() const;

private:
    PetData _pet;

    void decayHunger(unsigned long nowMs);
    void decayHappiness(unsigned long nowMs);
    void checkPoop(unsigned long nowMs);
    void checkSickness(unsigned long nowMs);
    void checkSleep(uint8_t currentHour);
    void checkDeath(unsigned long nowMs);
    void checkDisciplineCall(unsigned long nowMs);
    void checkEvolution(unsigned long nowMs);
    void checkCareWindow(unsigned long nowMs);
    void triggerAttention(AttentionType type, unsigned long nowMs);
    unsigned long effectiveHungerInterval() const;
    unsigned long effectiveHappyInterval() const;
};
