#include "pet.h"
#include "config.h"
#include <Arduino.h>

void PetManager::initNewEgg(unsigned long nowMs) {
    _pet = PetData();
    _pet.characterId = CharacterID::EGG;
    _pet.stage = LifeStage::EGG;
    _pet.stageStartMs = nowMs;
    _pet.lastHungerDecayMs = nowMs;
    _pet.lastHappyDecayMs  = nowMs;
    _pet.lastPoopMs        = nowMs;
    _pet.lastAgeTickMs     = nowMs;
    _pet.lastSickCheckMs   = nowMs;
    _pet.lastDisciplineMs  = nowMs;
}

void PetManager::loadFromSave(const PetData& d) {
    _pet = d;
}

PetData& PetManager::data() { return _pet; }
const PetData& PetManager::data() const { return _pet; }

unsigned long PetManager::effectiveHungerInterval() const {
    const auto& def = getCharacterDef(_pet.characterId);
    if (def.hungerDecayMul == 0) return HUNGER_DECAY_MS;
    return HUNGER_DECAY_MS * 10UL / def.hungerDecayMul;
}

unsigned long PetManager::effectiveHappyInterval() const {
    const auto& def = getCharacterDef(_pet.characterId);
    if (def.happyDecayMul == 0) return HAPPY_DECAY_MS;
    return HAPPY_DECAY_MS * 10UL / def.happyDecayMul;
}

void PetManager::update(unsigned long nowMs, uint8_t currentHour) {
    if (_pet.isDead) return;

    // Egg stage: just wait for hatch
    if (_pet.stage == LifeStage::EGG) {
        if (nowMs - _pet.stageStartMs >= EGG_HATCH_MS) {
            _pet.readyToEvolve = true;
        }
        return;
    }

    // Age tick
    if (nowMs - _pet.lastAgeTickMs >= AGE_TICK_MS) {
        _pet.age++;
        _pet.totalAge++;
        _pet.lastAgeTickMs = nowMs;
    }

    // Sleep check
    checkSleep(currentHour);
    if (_pet.isAsleep) return;

    // Stat decay
    decayHunger(nowMs);
    decayHappiness(nowMs);
    checkPoop(nowMs);
    checkSickness(nowMs);
    checkDisciplineCall(nowMs);
    checkCareWindow(nowMs);
    checkEvolution(nowMs);
    checkDeath(nowMs);
}

void PetManager::decayHunger(unsigned long nowMs) {
    unsigned long interval = effectiveHungerInterval();
    if (nowMs - _pet.lastHungerDecayMs >= interval) {
        if (_pet.hunger > 0) {
            _pet.hunger--;
        }
        _pet.lastHungerDecayMs = nowMs;
        if (_pet.hunger == 0 && _pet.pendingAttention == AttentionType::NONE) {
            triggerAttention(AttentionType::HUNGRY, nowMs);
        }
    }
}

void PetManager::decayHappiness(unsigned long nowMs) {
    unsigned long interval = effectiveHappyInterval();
    if (nowMs - _pet.lastHappyDecayMs >= interval) {
        if (_pet.happiness > 0) {
            _pet.happiness--;
        }
        _pet.lastHappyDecayMs = nowMs;
        if (_pet.happiness == 0 && _pet.pendingAttention == AttentionType::NONE) {
            triggerAttention(AttentionType::UNHAPPY, nowMs);
        }
    }
}

void PetManager::checkPoop(unsigned long nowMs) {
    unsigned long interval = (_pet.stage <= LifeStage::CHILD)
        ? POOP_INTERVAL_YOUNG_MS : POOP_INTERVAL_MS;
    if (nowMs - _pet.lastPoopMs >= interval) {
        if (_pet.poopCount < MAX_POOP) {
            _pet.poopCount++;
        }
        _pet.lastPoopMs = nowMs;
    }
}

void PetManager::checkSickness(unsigned long nowMs) {
    if (_pet.isSick) return;
    // Sick from too much poop
    if (_pet.poopCount >= 3) {
        if (nowMs - _pet.lastSickCheckMs >= SICK_FROM_POOP_MS) {
            _pet.isSick = true;
            _pet.sicknessLevel = 1 + (rand() % 3);  // 1-3 doses
            _pet.medicineGiven = 0;
            _pet.lastSickCheckMs = nowMs;
        }
    } else {
        _pet.lastSickCheckMs = nowMs;
    }
    // Old age sickness
    if (_pet.age >= 15 && ((_pet.age - 15) % 3 == 0) && !_pet.isSick) {
        if (rand() % 10 < 3) {  // 30% chance per age tick
            _pet.isSick = true;
            _pet.sicknessLevel = 2 + (rand() % 2);
            _pet.medicineGiven = 0;
        }
    }
}

void PetManager::checkDisciplineCall(unsigned long nowMs) {
    if (_pet.stage < LifeStage::CHILD) return;
    if (_pet.pendingAttention != AttentionType::NONE) return;
    if (_pet.discipline >= MAX_DISCIPLINE) return;

    if (nowMs - _pet.lastDisciplineMs >= DISCIPLINE_INTERVAL_MS) {
        if (rand() % 3 == 0) {  // 33% chance per interval
            triggerAttention(AttentionType::DISCIPLINE, nowMs);
            _pet.disciplineCalls++;
        }
        _pet.lastDisciplineMs = nowMs;
    }
}

void PetManager::checkSleep(uint8_t currentHour) {
    if (_pet.stage == LifeStage::EGG) return;
    const auto& def = getCharacterDef(_pet.characterId);

    bool shouldSleep;
    if (def.sleep.bedHour > def.sleep.wakeHour) {
        // Overnight: e.g. bed=22, wake=9 -> sleep if hour>=22 OR hour<9
        shouldSleep = (currentHour >= def.sleep.bedHour || currentHour < def.sleep.wakeHour);
    } else {
        shouldSleep = (currentHour >= def.sleep.bedHour && currentHour < def.sleep.wakeHour);
    }

    if (shouldSleep && !_pet.isAsleep) {
        _pet.isAsleep = true;
        _pet.lightOff = false;
        _pet.pendingAttention = AttentionType::SLEEP;
    } else if (!shouldSleep && _pet.isAsleep) {
        _pet.isAsleep = false;
        _pet.lightOff = false;
        _pet.pendingAttention = AttentionType::NONE;
    }
}

void PetManager::checkDeath(unsigned long nowMs) {
    // Death from prolonged hunger (care window x 8 = 2 hours)
    if (_pet.hunger == 0 && _pet.pendingAttention == AttentionType::HUNGRY) {
        if (nowMs - _pet.attentionStartMs >= CARE_WINDOW_MS * 8) {
            _pet.isDead = true;
            _pet.deathCause = 0;
            _pet.characterId = CharacterID::GHOST;
            _pet.stage = LifeStage::DEAD;
        }
    }
    // Death from prolonged sickness
    if (_pet.isSick) {
        if (nowMs - _pet.lastSickCheckMs >= CARE_WINDOW_MS * 12) {
            _pet.isDead = true;
            _pet.deathCause = 1;
            _pet.characterId = CharacterID::GHOST;
            _pet.stage = LifeStage::DEAD;
        }
    }
    // Death from old age (168 hours = 7 days)
    if (_pet.age >= 168) {
        _pet.isDead = true;
        _pet.deathCause = 2;
        _pet.characterId = CharacterID::GHOST;
        _pet.stage = LifeStage::DEAD;
    }
}

void PetManager::checkEvolution(unsigned long nowMs) {
    if (_pet.readyToEvolve) return;
    switch (_pet.stage) {
        case LifeStage::BABY:
            if (nowMs - _pet.stageStartMs >= BABY_EVOLVE_MS) {
                _pet.readyToEvolve = true;
            }
            break;
        case LifeStage::CHILD:
            if (_pet.age >= CHILD_EVOLVE_AGE) {
                _pet.readyToEvolve = true;
            }
            break;
        case LifeStage::TEEN:
            if (_pet.age >= TEEN_EVOLVE_AGE) {
                _pet.readyToEvolve = true;
            }
            break;
        case LifeStage::ADULT:
            if (isSecretEligible(_pet.totalCareMistakes, _pet.characterId, _pet.age)) {
                if (rand() % 10 < 2) {  // 20% chance per check
                    _pet.readyToEvolve = true;
                }
            }
            break;
        default:
            break;
    }
}

void PetManager::checkCareWindow(unsigned long nowMs) {
    if (_pet.pendingAttention == AttentionType::NONE) return;
    if (_pet.pendingAttention == AttentionType::SLEEP) return;
    if (_pet.pendingAttention == AttentionType::DISCIPLINE) return;

    if (nowMs - _pet.attentionStartMs >= CARE_WINDOW_MS) {
        _pet.careMistakes++;
        _pet.totalCareMistakes++;
        _pet.pendingAttention = AttentionType::NONE;
    }
}

void PetManager::triggerAttention(AttentionType type, unsigned long nowMs) {
    if (_pet.pendingAttention != AttentionType::NONE) return;
    _pet.pendingAttention = type;
    _pet.attentionStartMs = nowMs;
}

bool PetManager::hasAttention() const {
    return _pet.pendingAttention != AttentionType::NONE;
}

bool PetManager::isEvolving() const {
    return _pet.readyToEvolve;
}

void PetManager::doEvolve(unsigned long nowMs) {
    _pet.readyToEvolve = false;
    CharacterID next;

    if (_pet.stage == LifeStage::EGG) {
        next = CharacterID::BABY_CHAN;
        _pet.stage = LifeStage::BABY;
    } else if (_pet.stage == LifeStage::ADULT &&
               isSecretEligible(_pet.totalCareMistakes, _pet.characterId, _pet.age)) {
        next = CharacterID::SO_ARM;
        // stage stays ADULT
    } else {
        next = resolveEvolution(_pet.characterId, _pet.careMistakes, _pet.discipline);
        if (next == CharacterID::NONE) return;

        // Advance stage
        const auto& def = getCharacterDef(next);
        _pet.stage = def.stage;
    }

    _pet.characterId = next;
    const auto& newDef = getCharacterDef(next);
    _pet.weight = newDef.baseWeight > 0 ? newDef.baseWeight : _pet.weight;

    // Reset per-stage counters
    _pet.careMistakes = 0;
    _pet.discipline = _pet.discipline / 2;  // halve discipline on evolution
    _pet.disciplineCalls = 0;
    _pet.stageStartMs = nowMs;
    _pet.pendingAttention = AttentionType::NONE;
}

// === Player Actions ===

bool PetManager::feedMeal() {
    if (_pet.isAsleep || _pet.isDead) return false;
    if (_pet.hunger >= MAX_HUNGER) return false;
    if (_pet.pendingAttention == AttentionType::DISCIPLINE) return false;

    _pet.hunger++;
    _pet.weight = min((uint8_t)(_pet.weight + MEAL_WEIGHT), MAX_WEIGHT);
    if (_pet.pendingAttention == AttentionType::HUNGRY) {
        _pet.pendingAttention = AttentionType::NONE;
    }
    return true;
}

bool PetManager::feedSnack() {
    if (_pet.isAsleep || _pet.isDead) return false;
    if (_pet.happiness >= MAX_HAPPY) return false;

    _pet.happiness++;
    _pet.weight = min((uint8_t)(_pet.weight + SNACK_WEIGHT), MAX_WEIGHT);
    if (_pet.pendingAttention == AttentionType::UNHAPPY) {
        _pet.pendingAttention = AttentionType::NONE;
    }
    return true;
}

bool PetManager::startGame() {
    if (_pet.isAsleep || _pet.isDead) return false;
    if (_pet.pendingAttention == AttentionType::DISCIPLINE) return false;
    return true;
}

void PetManager::onGameWin() {
    if (_pet.happiness < MAX_HAPPY) _pet.happiness++;
    if (_pet.weight > MIN_WEIGHT) _pet.weight -= GAME_WEIGHT;
    if (_pet.pendingAttention == AttentionType::UNHAPPY) {
        _pet.pendingAttention = AttentionType::NONE;
    }
}

void PetManager::onGameLose() {
    if (_pet.weight > MIN_WEIGHT) _pet.weight -= GAME_WEIGHT;
}

bool PetManager::discipline() {
    if (_pet.pendingAttention != AttentionType::DISCIPLINE) return false;
    _pet.discipline = min((uint8_t)(_pet.discipline + DISCIPLINE_INC), MAX_DISCIPLINE);
    _pet.pendingAttention = AttentionType::NONE;
    return true;
}

bool PetManager::giveMedicine() {
    if (!_pet.isSick || _pet.isDead) return false;
    _pet.medicineGiven++;
    if (_pet.medicineGiven >= _pet.sicknessLevel) {
        _pet.isSick = false;
        _pet.sicknessLevel = 0;
        _pet.medicineGiven = 0;
    }
    return true;
}

bool PetManager::clean() {
    if (_pet.poopCount == 0 || _pet.isDead) return false;
    _pet.poopCount = 0;  // clean all at once
    if (_pet.pendingAttention == AttentionType::POOP) {
        _pet.pendingAttention = AttentionType::NONE;
    }
    return true;
}

bool PetManager::toggleLight() {
    if (!_pet.isAsleep) return false;
    _pet.lightOff = !_pet.lightOff;
    if (_pet.lightOff && _pet.pendingAttention == AttentionType::SLEEP) {
        _pet.pendingAttention = AttentionType::NONE;
    }
    return true;
}
