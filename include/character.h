#pragma once
#include <cstdint>

enum class CharacterID : uint8_t {
    NONE = 0,
    EGG,
    BABY_CHAN,       // ﾍﾞﾋﾞｰﾁｬﾝ
    CHIBI_STACK,     // ﾁﾋﾞｽﾀｯｸﾁｬﾝ
    STACK_JR,        // ｽﾀｯｸﾁｬﾝJr (good teen)
    DANBOARD_CHAN,   // ﾀﾞﾝﾎﾞｰﾙﾁｬﾝ (bad teen)
    AI_STACK,        // AIｽﾀｯｸﾁｬﾝ (best good)
    ROSTACK,         // ﾛｽﾀｯｸﾁｬﾝ (good)
    TAKAO,           // ﾀｶｵ版 (mid good)
    REXXCHAN,        // ﾚｯｸｽﾁｬﾝ (best bad)
    PROPELLA,        // ﾌﾟﾛﾍﾟﾗﾁｬﾝ (mid bad)
    DK_ATOM,         // DKｱﾄﾑﾁｬﾝ (worst bad)
    SO_ARM,          // SO-ARM-chan (secret)
    GHOST,           // death
    CHARACTER_COUNT
};

enum class LifeStage : uint8_t {
    EGG = 0,
    BABY,
    CHILD,
    TEEN,
    ADULT,
    DEAD
};

struct SleepSchedule {
    uint8_t bedHour;
    uint8_t wakeHour;
};

struct CharacterDef {
    CharacterID   id;
    const char*   nameJP;
    const char*   nameEN;
    LifeStage     stage;
    uint8_t       baseWeight;
    SleepSchedule sleep;
    uint8_t       hungerDecayMul;  // x10 (10=normal, 15=1.5x faster)
    uint8_t       happyDecayMul;
};

const CharacterDef& getCharacterDef(CharacterID id);
CharacterID resolveEvolution(CharacterID current, uint8_t careMistakes, uint8_t disciplinePct);
bool isSecretEligible(uint16_t totalCareMistakes, CharacterID currentChar, uint16_t age);
