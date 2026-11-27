#pragma once

#include "GameRules.h"

#include <array>
#include <cstddef>

enum class MinigameId : std::size_t {
    Slots,
    Blackjack,
    RabbitRacing,
    Dice,
    Count
};

enum class MarketItem {
    Magnet,
    Xray,
    RaceTip
};

class Player {
public:
    int carrots = GameRules::startingCarrots;
    int magnetCharges = 0;
    int xrayCharges = 0;
    int raceTipCharges = 0;
    int magnetStock = GameRules::marketMagnetStock;
    int xrayStock = GameRules::marketXrayStock;
    int raceTipStock = GameRules::marketRaceTipStock;
    bool emergencyUsed = false;
    std::array<bool, static_cast<std::size_t>(MinigameId::Count)> minigameWins{};

    void reset();
    void resetRun();
    void recordWin(MinigameId minigame);
    bool hasWon(MinigameId minigame) const;
    int slotPairPayout(int bet) const;
    bool buyMagnet();
    bool buyXray();
    bool buyRaceTip();
    bool useMagnet();
    bool useXray();
    bool useRaceTip();
    bool spendCarrots(int amount);
    void addCarrots(long long amount);
    bool claimEmergencyCarrots(MarketItem bonusItem);
};
