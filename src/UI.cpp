#include "UI.h"
#include <iostream>
#include <sstream>

void UI::clear() {
    std::cout << "\033[2J\033[3J\033[H" << std::flush;
}

void UI::drawHeader(const std::string& title) {
    std::cout << "╔════════════════════════════════════════════════════╗\n";
    std::cout << "║  " << title;
    std::size_t visibleCharacters = 0;
    for (unsigned char character : title) {
        if ((character & 0xC0) != 0x80) ++visibleCharacters;
    }
    for (std::size_t i = visibleCharacters; i < 50; ++i) std::cout << " ";
    std::cout << "║\n";
    std::cout << "╚════════════════════════════════════════════════════╝\n";
}

bool UI::readInt(const std::string& prompt, int& value, int minValue, int maxValue) {
    while (true) {
        std::cout << prompt;
        std::string line;
        if (!std::getline(std::cin, line)) return false;

        std::istringstream input(line);
        int parsedValue;
        char extraCharacter;
        if ((input >> parsedValue) && !(input >> extraCharacter)) {
            if (parsedValue >= minValue && parsedValue <= maxValue) {
                value = parsedValue;
                return true;
            }
            std::cout << "Введите число от " << minValue << " до " << maxValue << ".\n";
        } else {
            std::cout << "Некорректный ввод. Попробуйте ещё раз.\n";
        }
    }
}

void UI::pause(const std::string& prompt) {
    std::cout << '\n' << prompt;
    std::string line;
    std::getline(std::cin, line);
}

void UI::dialogue(const std::string& title, const std::string& text) {
    clear();
    drawHeader(title);
    std::cout << '\n' << text << '\n';
    pause("[ Enter — продолжить ]");
}
