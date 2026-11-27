#include "Minigames.h"

#include "UI.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <iostream>
#include <random>
#include <string>
#include <thread>
#include <vector>

namespace {
std::mt19937& randomEngine() {
    static std::mt19937 engine(std::random_device{}());
    return engine;
}

int randomInt(int minimum, int maximum) {
    std::uniform_int_distribution<int> distribution(minimum, maximum);
    return distribution(randomEngine());
}

std::vector<int> shuffledDeck() {
    std::vector<int> deck;
    for (int value = 2; value <= 9; ++value) {
        for (int suit = 0; suit < 4; ++suit) deck.push_back(value);
    }
    for (int card = 0; card < 16; ++card) deck.push_back(10);
    for (int ace = 0; ace < 4; ++ace) deck.push_back(11);
    std::shuffle(deck.begin(), deck.end(), randomEngine());
    return deck;
}

std::string cardName(int value) {
    return value == 11 ? "туз (11)" : std::to_string(value);
}

bool askBet(Player& player, const std::string& game, const std::string& rules,
            int& bet) {
    UI::clear();
    UI::drawHeader(game);
    std::cout << "\n" << rules << "\n"
              << "Баланс: " << player.carrots
              << " морковок. Можно поставить весь баланс; 0 — назад.\n\n";
    if (player.carrots < 1) {
        std::cout << "Для ставки нужна хотя бы 1 морковка.\n";
        UI::pause();
        return false;
    }
    if (!UI::readInt(" Ставка > ", bet, 0, player.carrots)) return false;
    return bet > 0;
}

void showRoundResult(const std::string& message, Player& player) {
    std::cout << "\n" << message << "\n"
              << "Баланс: " << player.carrots << " морковок.\n";
    UI::pause();
}

}

void Slots::play(Player& player) {
    int bet = 0;
    if (!askBet(player, "СЛОТЫ «МОРКОВКА УДАЧИ»",
                "Пара приносит чистую прибыль +50%; три одинаковых символа "
                "дают x6. Магнит может свести символы.", bet)) return;

    bool useMagnet = false;
    if (player.magnetCharges > 0) {
        UI::clear();
        UI::drawHeader("МАГНИТ СЛОТОВ");
        std::cout << "\nУ тебя " << player.magnetCharges
                  << " заряд(а) магнита. Он притянет один символ к паре.\n"
                  << "  1. Использовать\n  0. Сохранить\n\n";
        int choice = 0;
        if (!UI::readInt(" Твой выбор > ", choice, 0, 1)) return;
        useMagnet = choice == 1;
    }

    if (!player.spendCarrots(bet)) return;

    static const std::array<std::string, 4> symbols = {
        "🥕", "🐇", "⭐", "💎"
    };
    int result[3] = {randomInt(0, 3), randomInt(0, 3), randomInt(0, 3)};

    for (int frame = 0; frame < 4; ++frame) {
        UI::clear();
        UI::drawHeader("МОРКОВКА УДАЧИ");
        std::cout << "\n\n       ┌──────┬──────┬──────┐\n"
                  << "       │  " << symbols[randomInt(0, 3)] << "  │  "
                  << symbols[randomInt(0, 3)] << "  │  "
                  << symbols[randomInt(0, 3)] << "  │\n"
                  << "       └──────┴──────┴──────┘\n\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(170));
    }

    if (useMagnet && player.useMagnet()) {
        if (result[0] == result[1] || result[0] == result[2] ||
            result[1] == result[2]) {
            const int pairSymbol = result[0] == result[1] || result[0] == result[2]
                ? result[0] : result[1];
            result[randomInt(0, 2)] = pairSymbol;
        } else {
            result[2] = result[0];
        }
    }

    const bool triple = result[0] == result[1] && result[1] == result[2];
    const bool pair = result[0] == result[1] || result[0] == result[2] ||
                      result[1] == result[2];
    int payout = 0;
    if (triple) {
        payout = bet * 6;
    } else if (pair) {
        payout = player.slotPairPayout(bet);
    }

    UI::clear();
    UI::drawHeader("РЕЗУЛЬТАТ СЛОТОВ");
    std::cout << "\n       ┌──────┬──────┬──────┐\n"
              << "       │  " << symbols[result[0]] << "  │  "
              << symbols[result[1]] << "  │  "
              << symbols[result[2]] << "  │\n"
              << "       └──────┴──────┴──────┘\n";
    if (payout > 0) {
        player.addCarrots(payout);
        player.recordWin(MinigameId::Slots);
        showRoundResult(
            std::string(triple ? "Три одинаковых символа! " : "Пара! ") +
                "Возврат: " + std::to_string(payout) + " морковок.",
            player);
    } else {
        showRoundResult("Комбинации нет. Попробуй ещё раз.", player);
    }
}

void Blackjack::play(Player& player) {
    int bet = 0;
    if (!askBet(player, "БЛЭКДЖЕК",
                "Набери больше дилера, не превышая 21. Победа — x2; "
                "ничья возвращает ставку.", bet)) return;

    bool useXray = false;
    if (player.xrayCharges > 0) {
        UI::clear();
        UI::drawHeader("РЕНТГЕН");
        std::cout << "\nУ тебя " << player.xrayCharges
                  << " заряд(а). Рентген покажет следующую карту из колоды.\n"
                  << "  1. Использовать\n  0. Сохранить\n\n";
        int choice = 0;
        if (!UI::readInt(" Твой выбор > ", choice, 0, 1)) return;
        useXray = choice == 1;
    }
    if (!player.spendCarrots(bet)) return;

    const std::vector<int> deck = shuffledDeck();
    std::size_t nextCard = 0;
    const auto drawCard = [&deck, &nextCard]() { return deck[nextCard++]; };
    int playerTotal = drawCard() + drawCard();
    int dealerTotal = drawCard() + drawCard();
    UI::clear();
    UI::drawHeader("БЛЭКДЖЕК");
    if (useXray && player.useXray()) {
        std::cout << "\nРентген: следующая карта — "
                  << cardName(deck[nextCard]) << ".\n";
    }
    std::cout << "\nТвои карты: " << playerTotal << ".\n";

    while (playerTotal < 21) {
        std::cout << "\n  1. Взять карту\n  0. Остановиться\n";
        int choice = 0;
        if (!UI::readInt(" Твой выбор > ", choice, 0, 1)) return;
        if (choice == 0) break;
        const int card = drawCard();
        playerTotal += card;
        std::cout << "Карта: " << cardName(card) << ". Итого: "
                  << playerTotal << ".\n";
    }

    if (playerTotal <= 21) {
        while (dealerTotal < 17) dealerTotal += drawCard();
    }

    UI::clear();
    UI::drawHeader("ИТОГ БЛЭКДЖЕКА");
    std::cout << "\nТвои очки: " << playerTotal << "\n"
              << "Очки дилера: " << dealerTotal << "\n";

    if (playerTotal > 21) {
        showRoundResult("Перебор — раунд за дилером.", player);
    } else if (dealerTotal > 21 || playerTotal > dealerTotal) {
        const int payout = bet * 2;
        player.addCarrots(payout);
        player.recordWin(MinigameId::Blackjack);
        showRoundResult("Победа! Возврат: " + std::to_string(payout) + " морковок.",
                        player);
    } else if (playerTotal == dealerTotal) {
        player.addCarrots(bet);
        showRoundResult("Ничья — ставка возвращена.", player);
    } else {
        showRoundResult("Дилер ближе к 21. Ставка проиграна.", player);
    }
}

void RabbitRacing::play(Player& player) {
    struct Rabbit {
        const char* name;
        const char* style;
        int minimumSpeed;
        int maximumSpeed;
        int odds;
    };
    static const std::array<Rabbit, 4> rabbits = {{
        {"Снежок", "ровный темп", 2, 4, 2},
        {"Искра", "быстрый старт", 2, 5, 3},
        {"Пуля", "рискованный рывок", 1, 6, 4},
        {"Уголёк", "сильный финиш", 1, 5, 3}
    }};

    bool useTip = false;
    UI::clear();
    UI::drawHeader("СКАЧКИ КРОЛИКОВ");
    if (player.raceTipCharges > 0) {
        std::cout << "\nЗаписка букмекера покажет двух зайцев; один из них "
                     "точно победит.\n"
                  << "  1. Использовать записку\n"
                  << "  0. Не использовать\n\n";
        int tipChoice = 0;
        if (!UI::readInt(" Твой выбор > ", tipChoice, 0, 1)) return;
        useTip = tipChoice == 1;
    }

    std::vector<int> betOptions;
    UI::clear();
    UI::drawHeader(useTip ? "ЗАПИСКА БУКМЕКЕРА" : "СКАЧКИ КРОЛИКОВ");
    if (useTip) {
        const int first = randomInt(0, static_cast<int>(rabbits.size()) - 1);
        int second = randomInt(0, static_cast<int>(rabbits.size()) - 2);
        if (second >= first) ++second;
        betOptions = {first, second};
        std::cout << "\nВ записке два кандидата. Один из них точно придёт первым:\n";
    } else {
        for (std::size_t index = 0; index < rabbits.size(); ++index) {
            betOptions.push_back(static_cast<int>(index));
        }
        std::cout << "\nВыбери кролика. Выше множитель — рискованнее забег:\n";
    }

    for (std::size_t option = 0; option < betOptions.size(); ++option) {
        const Rabbit& rabbit = rabbits[static_cast<std::size_t>(betOptions[option])];
        std::cout << "  " << option + 1 << ". " << rabbit.name
                  << " — " << rabbit.style << ", выплата x"
                  << rabbit.odds << "\n";
    }
    std::cout << "  0. Назад\n\n";
    int choice = 0;
    if (!UI::readInt(" Твой выбор > ", choice, 0,
                     static_cast<int>(betOptions.size())) || choice == 0) {
        return;
    }
    const std::size_t selected =
        static_cast<std::size_t>(betOptions[static_cast<std::size_t>(choice - 1)]);

    int bet = 0;
    const std::string raceRules =
        std::string("Ты ставишь на ") + rabbits[selected].name +
        "; победа возвращает ставку с коэффициентом x" +
        std::to_string(rabbits[selected].odds) + ".";
    if (!askBet(player, "СТАВКА НА КРОЛИКА",
                raceRules, bet)) return;
    if (!player.spendCarrots(bet)) return;
    if (useTip && !player.useRaceTip()) return;

    std::vector<int> distances(rabbits.size(), 0);
    constexpr int finishLine = 30;
    std::size_t winner = static_cast<std::size_t>(betOptions.front());
    bool finished = false;
    for (int frame = 0; frame < finishLine && !finished; ++frame) {
        for (int index : betOptions) {
            const Rabbit& rabbit = rabbits[static_cast<std::size_t>(index)];
            distances[static_cast<std::size_t>(index)] +=
                randomInt(rabbit.minimumSpeed, rabbit.maximumSpeed);
        }
        for (int index : betOptions) {
            const std::size_t rabbitIndex = static_cast<std::size_t>(index);
            if (distances[rabbitIndex] >= finishLine &&
                (!finished || distances[rabbitIndex] > distances[winner])) {
                winner = rabbitIndex;
                finished = true;
            }
        }

        UI::clear();
        UI::drawHeader("ФИНИШНАЯ ПРЯМАЯ");
        std::cout << "\n";
        for (int index : betOptions) {
            const std::size_t rabbitIndex = static_cast<std::size_t>(index);
            const int position = std::min(distances[rabbitIndex], finishLine);
            std::cout << " " << rabbits[rabbitIndex].name << " |";
            for (int cell = 0; cell < position; ++cell) std::cout << '-';
            std::cout << "🐇";
            for (int cell = position; cell < finishLine; ++cell) std::cout << ' ';
            std::cout << "|\n";
        }
        std::cout << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(240));
    }

    UI::clear();
    UI::drawHeader("РЕЗУЛЬТАТ ЗАБЕГА");
    std::cout << "\nПобедитель: " << rabbits[winner].name << "!\n";
    if (winner == selected) {
        const int payout = bet * rabbits[selected].odds;
        player.addCarrots(payout);
        player.recordWin(MinigameId::RabbitRacing);
        showRoundResult("Твоя ставка выиграла. Возврат: " +
                            std::to_string(payout) + " морковок.",
                        player);
    } else {
        showRoundResult(std::string(rabbits[selected].name) +
                            " не успел к финишу. Ставка проиграна.",
                        player);
    }
}

void Dice::play(Player& player) {
    int bet = 0;
    if (!askBet(player, "КОСТИ: ЧЁТ ИЛИ НЕЧЁТ",
                "Угадай чётность суммы двух костей. Верный ответ возвращает x2.",
                bet)) return;

    UI::clear();
    UI::drawHeader("ТВОЯ СТАВКА");
    std::cout << "\n  1. Нечётная сумма\n  2. Чётная сумма\n  0. Назад\n\n";
    int choice = 0;
    if (!UI::readInt(" Твой выбор > ", choice, 0, 2) || choice == 0) return;
    if (!player.spendCarrots(bet)) return;

    int firstDie = 1;
    int secondDie = 1;
    for (int frame = 0; frame < 4; ++frame) {
        firstDie = randomInt(1, 6);
        secondDie = randomInt(1, 6);
        UI::clear();
        UI::drawHeader("БРОСОК КОСТЕЙ");
        std::cout << "\n          [" << firstDie << "]   [" << secondDie << "]\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(160));
    }

    const int sum = firstDie + secondDie;
    const bool even = sum % 2 == 0;
    UI::clear();
    UI::drawHeader("РЕЗУЛЬТАТ КОСТЕЙ");
    std::cout << "\nВыпало " << firstDie << " + " << secondDie << " = " << sum
              << (even ? " (чёт).\n" : " (нечёт).\n");
    if ((choice == 2) == even) {
        const int payout = bet * 2;
        player.addCarrots(payout);
        player.recordWin(MinigameId::Dice);
        showRoundResult("Угадал! Возврат: " + std::to_string(payout) + " морковок.",
                        player);
    } else {
        showRoundResult("Не угадал. Ставка проиграна.", player);
    }
}
