#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <array>

const char defeatSymbol = static_cast<char>(0xFF);

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

const std::vector<std::vector<int>> greenOrder = {
    {6, 4, 11, 1, 14, 7, 0, 10, 16, 5, 9, 2, 15, 8, 13, 3, 12, 17},
    {11, 2, 17, 0, 8, 13, 5, 16, 4, 14, 7, 1, 10, 3, 12, 6, 15, 9},
    {4, 11, 13, 14, 6, 3, 10, 7, 12, 1, 15, 9, 0, 16, 2, 8, 17, 5},
    {17, 0, 3, 4, 8, 13, 6, 12, 11, 10, 7, 15, 1, 16, 5, 9, 14, 2}
};

#endif // CONSTANTS_H