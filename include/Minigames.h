#pragma once

#include "Player.h"

class Minigame {
public:
    virtual void play(Player& player) = 0;
    virtual ~Minigame() = default;
};

class Slots : public Minigame {
public:
    void play(Player& player) override;
};

class Blackjack : public Minigame {
public:
    void play(Player& player) override;
};

class RabbitRacing : public Minigame {
public:
    void play(Player& player) override;
};

class Dice : public Minigame {
public:
    void play(Player& player) override;
};
