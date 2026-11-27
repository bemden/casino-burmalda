#pragma once

class Story {
public:
    static void prologue();
    static void dealerSlip();
    static void minigameClue(int minigame);
    static void dealerOffersCooperation();
    static void dealerPaymentNeeded(int remaining);
    static void emergencyHelp(const char* bonusItemName);
    static void dealerConfession();
    static void endingScreen();
    static void outOfCarrots();
};
