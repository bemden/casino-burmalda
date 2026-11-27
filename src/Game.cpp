#include "Game.h"

#include "Minigames.h"
#include "Story.h"
#include "UI.h"

#include <iostream>
#include <random>

namespace {
constexpr MinigameId clueGames[] = {
    MinigameId::Slots,
    MinigameId::Blackjack,
    MinigameId::RabbitRacing,
    MinigameId::Dice
};

MarketItem randomMarketItem() {
    static std::mt19937 engine(std::random_device{}());
    std::uniform_int_distribution<int> distribution(0, 2);
    return static_cast<MarketItem>(distribution(engine));
}

const char* marketItemName(MarketItem item) {
    switch (item) {
    case MarketItem::Magnet: return "магнит";
    case MarketItem::Xray: return "рентген";
    case MarketItem::RaceTip: return "записка букмекера";
    }
    return "предмет";
}

void showPurchaseResult(bool bought, const char* item) {
    if (bought) {
        std::cout << "\nКуплено: " << item << ".\n";
    } else {
        std::cout << "\nНе хватает морковок или товар закончился.\n";
    }
}
}

void Game::showStatus() const {
    std::cout << "\n  Морковки: " << player.carrots
              << "   Магниты: " << player.magnetCharges
              << "   Рентген: " << player.xrayCharges
              << "   Записки букмекера: " << player.raceTipCharges << "\n";
}

void Game::openBlackMarket() {
    while (true) {
        UI::clear();
        UI::drawHeader("ЧЁРНЫЙ РЫНОК МЯТЫ");
        showStatus();
        std::cout << "\n  1. Магнит слотов — "
                  << GameRules::magnetCost << " морковок, осталось "
                  << player.magnetStock
                  << " шт."
                  << "\n  2. Рентген: следующая карта в блэкджеке — "
                  << GameRules::xrayCost << " морковок, осталось "
                  << player.xrayStock << " шт."
                  << "\n  3. Записка букмекера: один из двух зайцев выиграет — "
                  << GameRules::raceTipCost << " морковок, осталось "
                  << player.raceTipStock << " шт."
                  << "\n  0. Назад\n\n";

        int choice = 0;
        if (!UI::readInt(" Твой выбор > ", choice, 0, 3) || choice == 0) return;
        if (choice == 1) {
            showPurchaseResult(player.buyMagnet(), "магнит");
        } else if (choice == 2) {
            showPurchaseResult(player.buyXray(), "рентген следующей карты");
        } else {
            showPurchaseResult(player.buyRaceTip(), "записка букмекера");
        }
        UI::pause();
    }
}

void Game::run() {
    player.reset();
    while (true) {
        Story::prologue();

        bool dealerQuestioned = false;
        bool dealerOfferShown = false;
        bool clueShown[4] = {false, false, false, false};
        bool finished = false;

        while (!finished) {
            if (player.carrots >= GameRules::dealerClueGoal &&
                !dealerQuestioned) {
                dealerQuestioned = true;
                Story::dealerSlip();
            }

            if (dealerQuestioned) {
                bool allCluesFound = true;
                for (std::size_t index = 0; index < 4; ++index) {
                    if (player.hasWon(clueGames[index])) {
                        if (!clueShown[index]) {
                            clueShown[index] = true;
                            Story::minigameClue(static_cast<int>(index));
                        }
                    } else {
                        allCluesFound = false;
                    }
                }

                if (allCluesFound && !dealerOfferShown) {
                    dealerOfferShown = true;
                    Story::dealerOffersCooperation();
                }
            }

            if (player.carrots == 0 && !player.emergencyUsed) {
                const MarketItem bonusItem = randomMarketItem();
                if (player.claimEmergencyCarrots(bonusItem)) {
                    Story::emergencyHelp(marketItemName(bonusItem));
                }
            }
            if (player.carrots == 0 && player.emergencyUsed) {
                Story::outOfCarrots();
                finished = true;
                break;
            }

            UI::clear();
            UI::drawHeader("КАЗИНО «БУРМАЛДА»");
            showStatus();
            std::cout << "\n  1. Слоты "
                      << (clueShown[0] ? "[✓]" : "[✗]")
                      << "\n  2. Блэкджек "
                      << (clueShown[1] ? "[✓]" : "[✗]")
                      << "\n  3. Скачки кроликов "
                      << (clueShown[2] ? "[✓]" : "[✗]")
                      << "\n  4. Кости "
                      << (clueShown[3] ? "[✓]" : "[✗]")
                      << "\n  5. Чёрный рынок"
                      << "\n";
            if (dealerOfferShown) {
                std::cout << "  6. Заплатить дилеру "
                          << GameRules::dealerCooperationCost
                          << " морковок\n";
            }
            std::cout << "  0. Завершить игру\n\n";

            int choice = 0;
            const int maximumChoice = dealerOfferShown ? 6 : 5;
            if (!UI::readInt(" Твой выбор > ", choice, 0, maximumChoice) ||
                choice == 0) {
                return;
            }
            if (choice == 5) {
                openBlackMarket();
                continue;
            }
            if (choice == 6 && dealerOfferShown) {
                if (player.carrots < GameRules::dealerCooperationCost) {
                    Story::dealerPaymentNeeded(
                        GameRules::dealerCooperationCost - player.carrots);
                    continue;
                }
                player.spendCarrots(GameRules::dealerCooperationCost);
                Story::dealerConfession();
                Story::endingScreen();
                return;
            }

            Slots slots;
            Blackjack blackjack;
            RabbitRacing race;
            Dice dice;
            switch (choice) {
            case 1: slots.play(player); break;
            case 2: blackjack.play(player); break;
            case 3: race.play(player); break;
            case 4: dice.play(player); break;
            default: break;
            }

            for (std::size_t index = 0; index < 4; ++index) {
                if (dealerQuestioned && player.hasWon(clueGames[index]) &&
                    !clueShown[index]) {
                    clueShown[index] = true;
                    Story::minigameClue(static_cast<int>(index));
                }
            }
        }

        UI::clear();
        UI::drawHeader("НОВАЯ ИГРА?");
        std::cout << "\n  1. Начать заново с 250 морковками\n";
        std::cout << "  0. Выйти\n\n";

        int choice = 0;
        if (!UI::readInt(" Твой выбор > ", choice, 0, 1) || choice == 0) return;
        player.reset();
    }
}
