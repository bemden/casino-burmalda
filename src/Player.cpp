#include "Player.h"

#include <algorithm>

void Player::reset() {
    resetRun();
}

void Player::resetRun() {
    carrots = GameRules::startingCarrots;
    magnetCharges = 0;
    xrayCharges = 0;
    raceTipCharges = 0;
    magnetStock = GameRules::marketMagnetStock;
    xrayStock = GameRules::marketXrayStock;
    raceTipStock = GameRules::marketRaceTipStock;
    emergencyUsed = false;
    minigameWins.fill(false);
}

void Player::recordWin(MinigameId minigame) {
    minigameWins[static_cast<std::size_t>(minigame)] = true;
}

bool Player::hasWon(MinigameId minigame) const {
    return minigameWins[static_cast<std::size_t>(minigame)];
}

int Player::slotPairPayout(int bet) const {
    if (bet <= 0) return 0;
    return bet + (bet + 1) / 2;
}

bool Player::buyMagnet() {
    if (magnetStock <= 0 || !spendCarrots(GameRules::magnetCost)) return false;
    --magnetStock;
    ++magnetCharges;
    return true;
}

bool Player::buyXray() {
    if (xrayStock <= 0 || !spendCarrots(GameRules::xrayCost)) return false;
    --xrayStock;
    ++xrayCharges;
    return true;
}

bool Player::buyRaceTip() {
    if (raceTipStock <= 0 || !spendCarrots(GameRules::raceTipCost)) return false;
    --raceTipStock;
    ++raceTipCharges;
    return true;
}

bool Player::useMagnet() {
    if (magnetCharges <= 0) return false;
    --magnetCharges;
    return true;
}

bool Player::useXray() {
    if (xrayCharges <= 0) return false;
    --xrayCharges;
    return true;
}

bool Player::useRaceTip() {
    if (raceTipCharges <= 0) return false;
    --raceTipCharges;
    return true;
}

bool Player::spendCarrots(int amount) {
    if (amount <= 0 || carrots < amount) return false;
    carrots -= amount;
    return true;
}

void Player::addCarrots(long long amount) {
    if (amount <= 0) return;
    carrots = static_cast<int>(std::min(
        static_cast<long long>(carrots) + amount,
        static_cast<long long>(GameRules::maxCarrots)));
}

bool Player::claimEmergencyCarrots(MarketItem bonusItem) {
    if (carrots != 0 || emergencyUsed) return false;
    emergencyUsed = true;
    addCarrots(GameRules::emergencyCarrots);
    switch (bonusItem) {
    case MarketItem::Magnet: ++magnetCharges; break;
    case MarketItem::Xray: ++xrayCharges; break;
    case MarketItem::RaceTip: ++raceTipCharges; break;
    }
    return true;
}
