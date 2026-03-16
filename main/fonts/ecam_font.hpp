#pragma once

#include "display/types.hpp"

namespace evms {

namespace Fonts {
    using EcamFontGlyph = Display::PixelMap<{ 12, 16 }>;

    const EcamFontGlyph& GetEcamFontGlyph(char character);
}

} // namespace evms
