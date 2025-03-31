#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <array>

constexpr std::array<char, 10> japaneseSymbols = {
    static_cast<char>(0xC0),
    static_cast<char>(0xD0),
    static_cast<char>(0xAE),
    static_cast<char>(0xD3),
    static_cast<char>(0xB7),
    static_cast<char>(0xFC),
    static_cast<char>(0xC2),
    static_cast<char>(0xD6),
    static_cast<char>(0xD1),
    static_cast<char>(0xD5)
};

constexpr std::array<char, 10> latinSymbols = {
    static_cast<char>(0x80),
    static_cast<char>(0x81),
    static_cast<char>(0x82),
    static_cast<char>(0xC6),
    static_cast<char>(0x46),
    static_cast<char>(0xAF),
    static_cast<char>(0xF8),
    static_cast<char>(0xFE),
    static_cast<char>(0xA4),
    static_cast<char>(0x56)
};

#endif // CONSTANTS_H