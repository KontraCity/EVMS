#pragma once

#include "display/types.hpp"

namespace evms {

namespace Fonts {
    struct EcamValue14 {
        using Glyph = Display::PixelMap<{ 10, 15 }>;
        static constexpr int Spacing = 2;
        static const Glyph& GetGlyph(char character);
    };
}

} // namespace evms
