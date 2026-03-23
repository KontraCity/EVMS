#pragma once

#include "display/types.hpp"

namespace evms {

namespace Fonts {
    struct EcamValue16 {
        using Glyph = Display::PixelMap<{ 12, 17 }>;
        static constexpr int Spacing = 2;
        static const Glyph& GetGlyph(char character);
    };
}

} // namespace evms
