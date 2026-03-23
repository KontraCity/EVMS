#pragma once

#include "display/types.hpp"

namespace evms {

namespace Graphics {
    using LabelsBitmapType = Display::PixelMap<{ 277, 153 }>;
    extern const LabelsBitmapType LabelsBitmap;

    using MapGaugeBitmapType = Display::PixelMap<{ 129, 100 }>;
    extern const MapGaugeBitmapType MapGaugeBitmap;

    using FfGaugeBitmapType = Display::PixelMap<{ 100, 70 }>;
    extern const FfGaugeBitmapType FfGaugeBitmap;

    using OilPressGaugeBitmapType = Display::PixelMap<{ 100, 70 }>;
    extern const OilPressGaugeBitmapType OilPressGaugeBitmap;

    using TempGaugeBitmapType = Display::PixelMap<{ 100, 65 }>;
    extern const TempGaugeBitmapType TempGaugeBitmap;

    using TempGaugeBoxBitmapType = Display::PixelMap<{ 55, 27 }>;
    extern const TempGaugeBoxBitmapType TempGaugeBoxBitmap;
}

} // namespace evms
