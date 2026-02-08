#include "character.h"

static const CharacterDef CHARACTER_TABLE[] = {
    // id                   nameJP                      nameEN            stage            wt  sleep    hMul hpMul
    {CharacterID::NONE,     "",                         "",               LifeStage::EGG,   0, {0, 0},   10, 10},
    {CharacterID::EGG,      "\xe3\x81\x9f\xe3\x81\xbe\xe3\x81\x94",
                            "Egg",            LifeStage::EGG,   0, {0, 0},    0,  0},
    // たまご
    {CharacterID::BABY_CHAN, "\xe3\x83\x99\xe3\x83\x93\xe3\x83\xbc\xe3\x83\x81\xe3\x83\xa3\xe3\x83\xb3",
                            "Baby-chan",       LifeStage::BABY,  5, {20, 9},  10, 10},
    // ベビーチャン
    {CharacterID::CHIBI_STACK,"\xe3\x83\x81\xe3\x83\x93\xe3\x82\xb9\xe3\x82\xbf\xe3\x83\x83\xe3\x82\xaf\xe3\x83\x81\xe3\x83\xa3\xe3\x83\xb3",
                            "Chibi Stack",    LifeStage::CHILD,10, {21, 9},  10, 10},
    // チビスタックチャン
    {CharacterID::STACK_JR,  "\xe3\x82\xb9\xe3\x82\xbf\xe3\x83\x83\xe3\x82\xaf\xe3\x83\x81\xe3\x83\xa3\xe3\x83\xb3Jr",
                            "Stack Jr.",      LifeStage::TEEN, 20, {22, 9},  12, 10},
    // スタックチャンJr
    {CharacterID::DANBOARD_CHAN,"\xe3\x83\x80\xe3\x83\xb3\xe3\x83\x9c\xe3\x83\xbc\xe3\x83\xab\xe3\x83\x81\xe3\x83\xa3\xe3\x83\xb3",
                            "Danboard-chan",  LifeStage::TEEN, 20, {22,10},  15, 12},
    // ダンボールチャン
    {CharacterID::AI_STACK,  "AI\xe3\x82\xb9\xe3\x82\xbf\xe3\x83\x83\xe3\x82\xaf\xe3\x83\x81\xe3\x83\xa3\xe3\x83\xb3",
                            "AI Stack-chan",  LifeStage::ADULT,30, {22, 9},  10,  8},
    // AIスタックチャン
    {CharacterID::ROSTACK,   "\xe3\x83\xad\xe3\x82\xb9\xe3\x82\xbf\xe3\x83\x83\xe3\x82\xaf\xe3\x83\x81\xe3\x83\xa3\xe3\x83\xb3",
                            "Rostack-chan",   LifeStage::ADULT,30, {22, 9},  12, 10},
    // ロスタックチャン
    {CharacterID::TAKAO,     "\xe3\x82\xbf\xe3\x82\xab\xe3\x82\xaa\xe7\x89\x88",
                            "Takao-ban",      LifeStage::ADULT,30, {22, 9},  14, 12},
    // タカオ版
    {CharacterID::REXXCHAN,  "\xe3\x83\xac\xe3\x83\x83\xe3\x82\xaf\xe3\x82\xb9\xe3\x83\x81\xe3\x83\xa3\xe3\x83\xb3",
                            "Rexx-chan",      LifeStage::ADULT,35, {23,10},  12, 10},
    // レックスチャン
    {CharacterID::PROPELLA,  "\xe3\x83\x97\xe3\x83\xad\xe3\x83\x9a\xe3\x83\xa9\xe3\x83\x81\xe3\x83\xa3\xe3\x83\xb3",
                            "Propella-chan",  LifeStage::ADULT,35, {23,11},  14, 14},
    // プロペラチャン
    {CharacterID::DK_ATOM,   "DK\xe3\x82\xa2\xe3\x83\x88\xe3\x83\xa0\xe3\x83\x81\xe3\x83\xa3\xe3\x83\xb3",
                            "DK Atom-chan",   LifeStage::ADULT,40, {23,11},  16, 16},
    // DKアトムチャン
    {CharacterID::SO_ARM,    "SO-ARM\xe3\x83\x81\xe3\x83\xa3\xe3\x83\xb3",
                            "SO-ARM-chan",    LifeStage::ADULT,25, {22, 9},   8,  6},
    // SO-ARMチャン
    {CharacterID::GHOST,     "\xe3\x82\xb4\xe3\x83\xbc\xe3\x82\xb9\xe3\x83\x88",
                            "Ghost",          LifeStage::DEAD,  0, {0, 0},    0,  0},
    // ゴースト
};

static const int TABLE_SIZE = sizeof(CHARACTER_TABLE) / sizeof(CHARACTER_TABLE[0]);

const CharacterDef& getCharacterDef(CharacterID id) {
    uint8_t idx = static_cast<uint8_t>(id);
    if (idx < TABLE_SIZE) return CHARACTER_TABLE[idx];
    return CHARACTER_TABLE[0];
}

CharacterID resolveEvolution(CharacterID current, uint8_t careMistakes, uint8_t disciplinePct) {
    switch (current) {
        case CharacterID::BABY_CHAN:
            return CharacterID::CHIBI_STACK;

        case CharacterID::CHIBI_STACK:
            return (careMistakes <= 1)
                ? CharacterID::STACK_JR
                : CharacterID::DANBOARD_CHAN;

        case CharacterID::STACK_JR:
            if (disciplinePct >= 75) return CharacterID::AI_STACK;
            if (disciplinePct >= 50) return CharacterID::ROSTACK;
            return CharacterID::TAKAO;

        case CharacterID::DANBOARD_CHAN:
            if (disciplinePct >= 75) return CharacterID::REXXCHAN;
            if (disciplinePct >= 50) return CharacterID::PROPELLA;
            return CharacterID::DK_ATOM;

        default:
            return CharacterID::NONE;
    }
}

bool isSecretEligible(uint16_t totalCareMistakes, CharacterID currentChar, uint16_t age) {
    return (totalCareMistakes == 0 &&
            currentChar == CharacterID::AI_STACK &&
            age >= 10);
}
