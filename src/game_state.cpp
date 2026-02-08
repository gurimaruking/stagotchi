#include "game_state.h"
#include "config.h"

void StateMachine::init() {
    _current = GameState::TITLE_SCREEN;
    _previous = GameState::TITLE_SCREEN;
}

void StateMachine::transition(GameState newState) {
    _previous = _current;
    _current = newState;
}

bool StateMachine::hasSaveData() {
    _prefs.begin(NVS_NAMESPACE, true);
    uint32_t magic = _prefs.getUInt("magic", 0);
    _prefs.end();
    return (magic == SAVE_MAGIC);
}

void StateMachine::saveGame(const PetData& pet) {
    _prefs.begin(NVS_NAMESPACE, false);
    _prefs.putUInt("magic", SAVE_MAGIC);
    _prefs.putUChar("version", SAVE_VERSION);
    _prefs.putBytes("petdata", &pet, sizeof(PetData));
    _prefs.putULong("save_ms", millis());
    _prefs.end();
}

bool StateMachine::loadGame(PetData& pet) {
    _prefs.begin(NVS_NAMESPACE, true);
    uint32_t magic = _prefs.getUInt("magic", 0);
    uint8_t ver = _prefs.getUChar("version", 0);
    if (magic != SAVE_MAGIC || ver != SAVE_VERSION) {
        _prefs.end();
        return false;
    }
    size_t len = _prefs.getBytes("petdata", &pet, sizeof(PetData));
    unsigned long savedMs = _prefs.getULong("save_ms", 0);
    _prefs.end();

    if (len != sizeof(PetData)) return false;

    // Recalibrate ALL timers to current millis()
    // IMPORTANT: After reboot, millis() resets to 0.
    // savedMs was from previous boot's millis(), so we CANNOT compute
    // meaningful elapsed time. Just reset everything to "now" and
    // preserve the pet's current stats as-is (no offline decay).
    unsigned long now = millis();
    pet.lastHungerDecayMs = now;
    pet.lastHappyDecayMs  = now;
    pet.lastPoopMs        = now;
    pet.lastAgeTickMs     = now;
    pet.lastSickCheckMs   = now;
    pet.lastDisciplineMs  = now;
    pet.stageStartMs      = now;
    pet.attentionStartMs  = now;

    // Clear any pending attention to prevent immediate death check
    // (attentionStartMs was from a previous boot and is now invalid)
    if (pet.pendingAttention == AttentionType::HUNGRY ||
        pet.pendingAttention == AttentionType::UNHAPPY ||
        pet.pendingAttention == AttentionType::SICK) {
        pet.pendingAttention = AttentionType::NONE;
    }

    // Make sure readyToEvolve is false on load to prevent immediate evolution
    pet.readyToEvolve = false;

    return true;
}

void StateMachine::clearSave() {
    _prefs.begin(NVS_NAMESPACE, false);
    _prefs.clear();
    _prefs.end();
}
