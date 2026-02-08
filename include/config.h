#pragma once
#include <M5Unified.h>

// ========== Screen Layout ==========
constexpr int SCREEN_W = 320;
constexpr int SCREEN_H = 240;

// Menu icon strip (top row)
constexpr int ICON_ROW_Y   = 4;
constexpr int ICON_SIZE    = 24;
constexpr int ICON_GAP     = 16;
constexpr int ICON_COUNT   = 7;
constexpr int ICON_START_X = 28;  // (320 - 7*24 - 6*16) / 2
constexpr int ICON_STEP    = 40;  // 24 + 16

// Pet viewport
constexpr int PET_AREA_X = 60;
constexpr int PET_AREA_Y = 36;
constexpr int PET_AREA_W = 200;
constexpr int PET_AREA_H = 160;

// Status bar
constexpr int STATUS_BAR_Y = 204;
constexpr int STATUS_BAR_H = 36;

// Sprite size
constexpr int SPRITE_W = 48;
constexpr int SPRITE_H = 48;

// ========== Game Timing (milliseconds) ==========
constexpr unsigned long EGG_HATCH_MS          = 10UL * 1000;          // 10 sec
constexpr unsigned long BABY_EVOLVE_MS        = 65UL * 60 * 1000;    // 65 min
constexpr unsigned long CHILD_EVOLVE_AGE      = 3;   // age hours
constexpr unsigned long TEEN_EVOLVE_AGE       = 6;
constexpr unsigned long SECRET_EVOLVE_AGE     = 10;
constexpr unsigned long CARE_WINDOW_MS        = 15UL * 60 * 1000;    // 15 min
constexpr unsigned long DISCIPLINE_INTERVAL_MS= 3UL * 60 * 60 * 1000;// ~3 hours

// Stat decay base intervals
constexpr unsigned long HUNGER_DECAY_MS       = 60UL * 60 * 1000;    // 1 heart/hour
constexpr unsigned long HAPPY_DECAY_MS        = 50UL * 60 * 1000;    // 1 heart/50min
constexpr unsigned long POOP_INTERVAL_MS      = 180UL * 60 * 1000;   // 3 hours
constexpr unsigned long POOP_INTERVAL_YOUNG_MS= 90UL * 60 * 1000;    // 1.5 hours
constexpr unsigned long AGE_TICK_MS           = 60UL * 60 * 1000;    // 1 hour
constexpr unsigned long SICK_FROM_POOP_MS     = 30UL * 60 * 1000;    // 30 min poop->sick

// ========== Stat Limits ==========
constexpr uint8_t MAX_HUNGER     = 4;
constexpr uint8_t MAX_HAPPY      = 4;
constexpr uint8_t MAX_DISCIPLINE = 100;
constexpr uint8_t DISCIPLINE_INC = 25;
constexpr uint8_t MAX_POOP       = 4;
constexpr uint8_t MAX_MEDICINE   = 3;
constexpr uint8_t MIN_WEIGHT     = 5;
constexpr uint8_t MAX_WEIGHT     = 99;
constexpr uint8_t MEAL_WEIGHT    = 1;
constexpr uint8_t SNACK_WEIGHT   = 2;
constexpr uint8_t GAME_WEIGHT    = 1;

// ========== Save Data ==========
constexpr const char* NVS_NAMESPACE = "stagotchi";
constexpr uint32_t SAVE_MAGIC      = 0x53544147;  // "STAG"
constexpr uint8_t  SAVE_VERSION    = 1;

// ========== Autosave ==========
constexpr unsigned long AUTOSAVE_INTERVAL_MS = 60000;  // 60 sec

// ========== Colors (RGB565) ==========
constexpr uint16_t COL_BG        = 0xCE59;  // light greenish (Tamagotchi LCD feel)
constexpr uint16_t COL_PET_BG    = 0xD6BA;  // pet area background
constexpr uint16_t COL_BLACK     = 0x0000;
constexpr uint16_t COL_WHITE     = 0xFFFF;
constexpr uint16_t COL_DARK      = 0x2104;
constexpr uint16_t COL_ICON_BG   = 0x7BCF;  // icon row background
constexpr uint16_t COL_ICON_SEL  = 0x001F;  // selected icon highlight (blue)
constexpr uint16_t COL_STATUS_BG = 0x4208;  // status bar background
constexpr uint16_t COL_STATUS_FG = 0xFFFF;
constexpr uint16_t COL_HEART     = 0xF800;  // red
constexpr uint16_t COL_HEART_E   = 0x8410;  // empty heart gray
constexpr uint16_t COL_POOP      = 0x8200;  // brown
constexpr uint16_t COL_SICK      = 0x780F;  // purple
