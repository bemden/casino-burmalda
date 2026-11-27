#pragma once

#include "Player.h"

class Game {
public:
    void run();

private:
    Player player;
    void openBlackMarket();
    void showStatus() const;
};
