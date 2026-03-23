#pragma once

#include "display/types.hpp"

namespace evms {

namespace App {
    using CrossBitmapType = Display::PixelMap<{ 35, 35 }>;
    extern const CrossBitmapType CrossBitmap;

    using CalibrationMessageBitmapType = Display::PixelMap<{ 168, 51 }>;
    extern const CalibrationMessageBitmapType CalibrationMessageBitmap;

    using PleaseTouchAsInstructedMessageBitmapType = Display::PixelMap<{ 168, 12 }>;
    extern const PleaseTouchAsInstructedMessageBitmapType PleaseTouchAsInstructedMessageBitmap;

    using RecordedPleaseReleaseMessageBitmapType = Display::PixelMap<{ 160, 14 }>;
    extern const RecordedPleaseReleaseMessageBitmapType RecordedPleaseReleaseMessageBitmap;

    using ThankYouMessageBitmapType = Display::PixelMap<{ 66, 14 }>;
    extern const ThankYouMessageBitmapType ThankYouMessageBitmap;

    using TouchMessageBitmapType = Display::PixelMap<{ 22, 14 }>;
    extern const TouchMessageBitmapType FirstTouchMessageBitmap;
    extern const TouchMessageBitmapType SecondTouchMessageBitmap;
    extern const TouchMessageBitmapType ThirdTouchMessageBitmap;
    extern const TouchMessageBitmapType FourthTouchMessageBitmap;
}

} // namespace evms
