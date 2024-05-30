#pragma once

// STL modules
#include <cmath>
#include <cstdio>

// Custom modules
#include "display/master.hpp"

namespace kc {

namespace Display
{
    class Ui : public Master
    {
    private:
        static constexpr StaticBitMap<25, 22> CelciusDegreesIcon = {
            std::bitset<25>(0b0001111100000000111111100),
            std::bitset<25>(0b0011000110000011111111111),
            std::bitset<25>(0b0010000010000111110000111),
            std::bitset<25>(0b0010000010001111000000000),
            std::bitset<25>(0b0010000010001110000000000),
            std::bitset<25>(0b0011000110011110000000000),
            std::bitset<25>(0b0001111100011100000000000),
            std::bitset<25>(0b0000000000011100000000000),
            std::bitset<25>(0b0000000000011100000000000),
            std::bitset<25>(0b0000000000111000000000000),
            std::bitset<25>(0b0000000000111000000000000),
            std::bitset<25>(0b0000000000111000000000000),
            std::bitset<25>(0b0000000000111100000000000),
            std::bitset<25>(0b0000000000011100000000000),
            std::bitset<25>(0b0000000000011100000000000),
            std::bitset<25>(0b0000000000011100000000000),
            std::bitset<25>(0b0000000000001110000000000),
            std::bitset<25>(0b0000000000001111000000000),
            std::bitset<25>(0b0000000000000111000000000),
            std::bitset<25>(0b0000000000000111111000111),
            std::bitset<25>(0b0000000000000011111111111),
            std::bitset<25>(0b0000000000000000111111100),
        };

    private:
        /// @brief Create a "sliding value" widget
        /// @param value The value to create widget for
        /// @return Created widget
        static BitMap SlidingValueWidget(float value);

    public:
        /// @brief Initialize display UI
        /// @param sclPin Pin I2C bus clock wire is connected to
        /// @param sdaPin Pin I2C bus data wire is connected to
        Ui(int sclPin, int sdaPin);

        /// @brief Start display UI
        void start();
    };
}

} // namespace kc
