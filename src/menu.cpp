#include "menu.h"

void MenuSystem::init() {
    _cursor = 0;
    _open = false;
    _subCursor = 0;
}

void MenuSystem::moveCursorLeft() {
    if (_cursor > 0) _cursor--;
    else _cursor = static_cast<uint8_t>(MenuItem::MENU_COUNT) - 1;
}

void MenuSystem::moveCursorRight() {
    _cursor++;
    if (_cursor >= static_cast<uint8_t>(MenuItem::MENU_COUNT)) _cursor = 0;
}

void MenuSystem::open() {
    _open = true;
    _subCursor = 0;
}

void MenuSystem::close() {
    _open = false;
    _subCursor = 0;
}

void MenuSystem::feedSubUp() {
    if (_subCursor > 0) _subCursor--;
    else _subCursor = static_cast<uint8_t>(FeedChoice::FEED_COUNT) - 1;
}

void MenuSystem::feedSubDown() {
    _subCursor++;
    if (_subCursor >= static_cast<uint8_t>(FeedChoice::FEED_COUNT)) _subCursor = 0;
}
