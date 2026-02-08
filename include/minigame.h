#pragma once
#include <cstdint>

class MiniGame {
public:
    void start();
    void guessHigher();
    void guessLower();
    bool isFinished() const { return _finished; }
    bool isWin() const { return _wins >= 3; }

    uint8_t currentRound() const { return _round; }
    uint8_t currentNumber() const { return _currentNum; }
    uint8_t wins() const { return _wins; }
    uint8_t lastResult() const { return _lastResult; }
    bool showingResult() const { return _showResult; }

    void update(unsigned long nowMs);

private:
    uint8_t _round      = 1;
    uint8_t _wins       = 0;
    uint8_t _currentNum = 5;
    uint8_t _nextNum    = 0;
    uint8_t _lastResult = 0;  // 0=pending, 1=correct, 2=wrong
    bool    _showResult = false;
    bool    _finished   = false;
    unsigned long _resultShowMs = 0;

    void generateNext();
    void advanceRound();
};
