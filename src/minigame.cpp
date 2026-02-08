#include "minigame.h"
#include <Arduino.h>

void MiniGame::start() {
    _round = 1;
    _wins = 0;
    _currentNum = 1 + (rand() % 9);
    _lastResult = 0;
    _showResult = false;
    _finished = false;
    generateNext();
}

void MiniGame::generateNext() {
    _nextNum = 1 + (rand() % 9);
    // Avoid same number
    while (_nextNum == _currentNum) {
        _nextNum = 1 + (rand() % 9);
    }
}

void MiniGame::guessHigher() {
    if (_showResult || _finished) return;
    bool correct = (_nextNum > _currentNum);
    _lastResult = correct ? 1 : 2;
    if (correct) _wins++;
    _showResult = true;
    _resultShowMs = millis();
}

void MiniGame::guessLower() {
    if (_showResult || _finished) return;
    bool correct = (_nextNum < _currentNum);
    _lastResult = correct ? 1 : 2;
    if (correct) _wins++;
    _showResult = true;
    _resultShowMs = millis();
}

void MiniGame::advanceRound() {
    _currentNum = _nextNum;
    _lastResult = 0;
    _showResult = false;
    _round++;
    if (_round > 5) {
        _finished = true;
    } else {
        generateNext();
    }
}

void MiniGame::update(unsigned long nowMs) {
    if (_showResult && nowMs - _resultShowMs > 1200) {
        advanceRound();
    }
}
