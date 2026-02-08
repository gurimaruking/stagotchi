#pragma once
#include <cstdint>

enum class MenuItem : uint8_t {
    FEED = 0,
    LIGHT,
    PLAY,
    MEDICINE,
    CLEAN,
    STATUS,
    DISCIPLINE,
    MENU_COUNT  // = 7
};

enum class FeedChoice : uint8_t {
    MEAL = 0,
    SNACK,
    CANCEL,
    FEED_COUNT
};

class MenuSystem {
public:
    void init();

    void moveCursorLeft();
    void moveCursorRight();
    void open();
    void close();

    uint8_t getCursor() const { return _cursor; }
    MenuItem getCurrentItem() const { return static_cast<MenuItem>(_cursor); }
    bool isOpen() const { return _open; }

    // Feed submenu
    void feedSubUp();
    void feedSubDown();
    FeedChoice getFeedChoice() const { return static_cast<FeedChoice>(_subCursor); }
    uint8_t getSubCursor() const { return _subCursor; }

private:
    uint8_t _cursor    = 0;
    bool    _open      = false;
    uint8_t _subCursor = 0;
};
