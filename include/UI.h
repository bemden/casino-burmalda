#pragma once
#include <string>

class UI {
public:
    static void clear();
    static void drawHeader(const std::string& title);
    static bool readInt(const std::string& prompt, int& value, int minValue, int maxValue);
    static void pause(const std::string& prompt = "[ Нажмите Enter, чтобы продолжить... ]");
    static void dialogue(const std::string& title, const std::string& text);
};
