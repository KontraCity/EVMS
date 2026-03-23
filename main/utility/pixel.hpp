#pragma once

#include <cstdint>

namespace evms {

namespace Utility {
    constexpr uint16_t From888To565RGB(uint8_t r, uint8_t g, uint8_t b) {
        return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
    }

    constexpr uint16_t Blend565RGB(uint16_t background, uint16_t foreground, uint8_t alpha) {
        uint8_t backgroundR = (background >> 11) & 0x1F;
        uint8_t backgroundG = (background >> 5)  & 0x3F;
        uint8_t backgroundB = background         & 0x1F;

        uint8_t foregroundR = (foreground >> 11) & 0x1F;
        uint8_t foregroundG = (foreground >> 5)  & 0x3F;
        uint8_t foregroundB = foreground         & 0x1F;

        backgroundR = backgroundR + ((foregroundR - backgroundR) * alpha >> 8);
        backgroundG = backgroundG + ((foregroundG - backgroundG) * alpha >> 8);
        backgroundB = backgroundB + ((foregroundB - backgroundB) * alpha >> 8);

        return (backgroundR << 11) | (backgroundG << 5) | backgroundB;
    }
}

} // namespace evms
