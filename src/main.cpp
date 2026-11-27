#include "Game.h"
#include <algorithm>
#include <clocale>
#include <iostream>
#include <streambuf>
#include <string>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>

namespace {
class Utf8ConsoleBuffer : public std::streambuf {
public:
    Utf8ConsoleBuffer(HANDLE console, std::streambuf* fallback)
        : console_(console), fallback_(fallback) {
        DWORD mode = 0;
        attached_ = console_ != INVALID_HANDLE_VALUE && GetConsoleMode(console_, &mode);
        if (attached_) {
            SetConsoleMode(console_, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
        }
    }

    bool attached() const {
        return attached_;
    }

protected:
    int_type overflow(int_type character) override {
        if (traits_type::eq_int_type(character, traits_type::eof())) {
            return sync() == 0 ? traits_type::not_eof(character) : traits_type::eof();
        }

        const char byte = traits_type::to_char_type(character);
        return xsputn(&byte, 1) == 1 ? character : traits_type::eof();
    }

    std::streamsize xsputn(const char* data, std::streamsize length) override {
        if (!attached_) return fallback_->sputn(data, length);
        pending_.append(data, static_cast<std::size_t>(length));
        return writeCompleteCharacters() ? length : 0;
    }

    int sync() override {
        if (!attached_) return fallback_->pubsync();
        return writeCompleteCharacters() ? 0 : -1;
    }

private:
    bool writeCompleteCharacters() {
        std::wstring wide;
        std::size_t consumed = 0;

        while (consumed < pending_.size()) {
            const auto first = static_cast<unsigned char>(pending_[consumed]);
            std::size_t length = 1;
            unsigned int codePoint = first;

            if (first >= 0xC2 && first <= 0xDF) {
                length = 2;
                codePoint = first & 0x1F;
            } else if (first >= 0xE0 && first <= 0xEF) {
                length = 3;
                codePoint = first & 0x0F;
            } else if (first >= 0xF0 && first <= 0xF4) {
                length = 4;
                codePoint = first & 0x07;
            } else if (first >= 0x80) {
                wide.push_back(L'?');
                ++consumed;
                continue;
            }

            if (pending_.size() - consumed < length) break;

            bool valid = true;
            for (std::size_t i = 1; i < length; ++i) {
                const auto continuation =
                    static_cast<unsigned char>(pending_[consumed + i]);
                if ((continuation & 0xC0) != 0x80) {
                    valid = false;
                    break;
                }
                codePoint = (codePoint << 6) | (continuation & 0x3F);
            }

            if (!valid ||
                (length == 3 && first == 0xE0 &&
                 static_cast<unsigned char>(pending_[consumed + 1]) < 0xA0) ||
                (length == 3 && first == 0xED &&
                 static_cast<unsigned char>(pending_[consumed + 1]) >= 0xA0) ||
                (length == 4 && first == 0xF0 &&
                 static_cast<unsigned char>(pending_[consumed + 1]) < 0x90) ||
                (length == 4 && first == 0xF4 &&
                 static_cast<unsigned char>(pending_[consumed + 1]) > 0x8F)) {
                wide.push_back(L'?');
                ++consumed;
                continue;
            }

            if (codePoint <= 0xFFFF) {
                wide.push_back(static_cast<wchar_t>(codePoint));
            } else {
                codePoint -= 0x10000;
                wide.push_back(static_cast<wchar_t>(0xD800 + (codePoint >> 10)));
                wide.push_back(static_cast<wchar_t>(0xDC00 + (codePoint & 0x3FF)));
            }
            consumed += length;
        }

        if (consumed == 0) return true;

        std::size_t writtenTotal = 0;
        while (writtenTotal < wide.size()) {
            DWORD written = 0;
            const DWORD remaining = static_cast<DWORD>(
                std::min(wide.size() - writtenTotal,
                         static_cast<std::size_t>(MAXDWORD)));
            if (!WriteConsoleW(console_, wide.data() + writtenTotal, remaining,
                               &written, nullptr) || written == 0) {
                return false;
            }
            writtenTotal += written;
        }

        pending_.erase(0, consumed);
        return true;
    }

    HANDLE console_;
    std::streambuf* fallback_;
    std::string pending_;
    bool attached_ = false;
};

class WindowsConsole {
public:
    WindowsConsole()
        : buffer_(GetStdHandle(STD_OUTPUT_HANDLE), std::cout.rdbuf()) {
        if (buffer_.attached()) previous_ = std::cout.rdbuf(&buffer_);
    }

    ~WindowsConsole() {
        if (previous_) {
            std::cout.flush();
            std::cout.rdbuf(previous_);
        }
    }

private:
    Utf8ConsoleBuffer buffer_;
    std::streambuf* previous_ = nullptr;
};
}
#endif

int main() {
#ifdef _WIN32
    WindowsConsole console;
#endif
    setlocale(LC_ALL, "");
    Game game;
    game.run();
    return 0;
}

