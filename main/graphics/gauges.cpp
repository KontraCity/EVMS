#include "gauges.hpp"

#include <cmath>
#include <cstdio>

#include "graphics/bitmaps.hpp"
#include "graphics/shapes.hpp"
#include "utility/pixel.hpp"
#include "utility/math.hpp"
#include "fonts/ecam_value_14.hpp"
#include "fonts/ecam_value_16.hpp"

namespace evms {

namespace Colors {
    constexpr uint16_t EcamGreen = Utility::From888To565RGB(0, 186, 49);
    constexpr uint16_t EcamRed = Utility::From888To565RGB(165, 0, 0);
}

template <typename Font>
static Display::Size GetTextSize(const std::string& text) {
    Display::Size size = Font::Glyph::MapSize;
    size.width = (Font::Glyph::MapWidth + Font::Spacing) * text.length();
    return size;
}

template <typename Font>
static void DrawText(Display::Screen& screen, int x, int y, const char* text, bool fromTopRightCorner = false) {
    if (fromTopRightCorner) {
        Display::Size textSize = GetTextSize<Font>(text);
        x -= textSize.width;
    }

    for (const char* character = text; *character != '\0'; ++character) {
        screen.draw(x, y, Font::GetGlyph(*character));
        x += Font::Glyph::MapWidth + Font::Spacing;
    }
}

static void DrawValueText(Display::Screen& screen, int x, int y, float value, int intergerPartWidth = 3, bool withFractionalPart = true) {
    int scaled = static_cast<int>(std::round(value * 10.0f));
    int integerPart = scaled / 10;
    int fractionalPart = scaled % 10;

    char integerPartString[16] = {};
    std::snprintf(integerPartString, sizeof(integerPartString), "%*d", intergerPartWidth, integerPart);
    DrawText<Fonts::EcamValue16>(screen, x, y, integerPartString, true);

    if (withFractionalPart) {
        char fractionalPartString[3] = {};
        fractionalPartString[0] = '.';
        fractionalPartString[1] = '0' + std::abs(fractionalPart);
        fractionalPartString[2] = '\0';
        DrawText<Fonts::EcamValue14>(screen, x - 3, y + 2, fractionalPartString);
    }
}

static uint16_t GetColor(float value, float threshold, bool higherIsBad  = true) {
    if (higherIsBad)
        return value > threshold ? Colors::EcamRed : Colors::EcamGreen;
    return value < threshold ? Colors::EcamRed : Colors::EcamGreen;
}

static const char* GearIndexToChar(int gear) {
    switch (gear) {
        case -2: return "P";
        case -1: return "R";
        case  0: return "N";
        case  1: return "1";
        case  2: return "2";
        case  3: return "3";
        case  4: return "4";
        case  5: return "5";
        case  6: return "6";
        case  7: return "7";
        case  8: return "8";
        default: return "U";
    }
}

void Graphics::DrawLabels(Display::Screen& screen) {
    constexpr int X = 34, Y = 81;
    screen.draw(X, Y, LabelsBitmap);
}

void Graphics::DrawManifoldAbsolutePressGauge(Display::Screen& screen, float pressure) {
    constexpr int X = 60, Y = 63;
    pressure = Utility::Clamp(pressure, 0.0f, 9.999f);

    float angle = Utility::Map(Utility::Clamp(pressure, 0.975f, 1.95f), 1.0f, 1.9f, 145.0f, 355.0f);
    screen.draw(X - 60, Y - 60, MapGaugeBitmap);
    DrawDirectedLine(screen, X, Y, angle, 0.0f, 58.0f, Colors::EcamGreen, 3);

    int scaled = static_cast<int>(std::round(pressure * 1000.0f));
    int integerPart = scaled / 1000;
    int fractionalPart = scaled % 1000;

    char integerPartString[16] = {};
    std::snprintf(integerPartString, sizeof(integerPartString), "%d", integerPart);
    DrawText<Fonts::EcamValue16>(screen, X - 2, Y + 17, integerPartString);

    char fractionalPartString[16] = {};
    std::snprintf(fractionalPartString, sizeof(fractionalPartString), ".%03d", fractionalPart);
    DrawText<Fonts::EcamValue16>(screen, X + 10, Y + 17, fractionalPartString);
}

void Graphics::DrawFuelFlowGauge(Display::Screen& screen, float fuelFlow) {
    constexpr int X = 173, Y = 52;
    fuelFlow = Utility::Clamp(fuelFlow, 0.0f, 999.9f);

    float angle = Utility::Map(fuelFlow, 0.0f, 22.5f, 160.0f, 360.0f, true);
    screen.draw(X - 50, Y - 48, FfGaugeBitmap);
    DrawDirectedLine(screen, X, Y, angle, 0.0f, 45.0f, GetColor(fuelFlow, 20.0f), 3);
    DrawValueText(screen, X + 25, Y + 5, fuelFlow, 0);
}

void Graphics::DrawOilPressGauge(Display::Screen& screen, float pressure) { 
    constexpr int X = 269, Y = 52;
    pressure = Utility::Clamp(pressure, 0.0f, 999.9f);
    
    float angle = Utility::Map(pressure, 0.25f, 7.0f, 160.0f, 360.0f, true);
    screen.draw(X - 50, Y - 48, OilPressGaugeBitmap);
    DrawDirectedLine(screen, X, Y, angle, 0.0f, 45.0f, GetColor(pressure, 1.0f, false), 3);
    DrawValueText(screen, X + 25, Y + 5, pressure, 0);
}

void Graphics::DrawGearText(Display::Screen& screen, int gear) {
    constexpr int X = 82, Y = 125;
    DrawText<Fonts::EcamValue16>(screen, X, Y, GearIndexToChar(gear));
}

void Graphics::DrawEngineLoadText(Display::Screen& screen, float load) {
    constexpr int X = 160, Y = 125;
    DrawValueText(screen, X, Y, Utility::Clamp(load, 0.0f, 100.0f));
}

void Graphics::DrawIntakeAirTempText(Display::Screen& screen, float temperature) {
    constexpr int X = 232, Y = 125;
    DrawValueText(screen, X, Y, Utility::Clamp(temperature, -99.9f, 999.9f));
}

void Graphics::DrawBatteryVoltageText(Display::Screen& screen, float voltage) {
    constexpr int X = 291, Y = 125;
    DrawValueText(screen, X, Y, Utility::Clamp(voltage, 0.0f, 99.9f), 2);
}

void Graphics::DrawCoolantTempGauge(Display::Screen& screen, float temp) {
    constexpr int X = 60, Y = 198;
    temp = Utility::Clamp(temp, -999.0f, 999.9f);

    float angle = Utility::Map(temp, 45.0f, 135.0f, 180.0f, 360.0f, true);
    screen.draw(X - 50, Y - 48, TempGaugeBitmap);
    DrawDirectedLine(screen, X, Y, angle, 20.0f, 45.0f, GetColor(temp, 112.5f), 3);
    screen.draw(X - 27, Y - 11, TempGaugeBoxBitmap);
    DrawValueText(screen, X + 23, Y - 5, temp, 3, false);
}

void Graphics::DrawOilTempGauge(Display::Screen& screen, float temp) {
    constexpr int X = 160, Y = 198;
    temp = Utility::Clamp(temp, -999.0f, 999.9f);

    float angle = Utility::Map(temp, 50.0f, 150.0f, 180.0f, 360.0f, true);
    screen.draw(X - 50, Y - 48, TempGaugeBitmap);
    DrawDirectedLine(screen, X, Y, angle, 20.0f, 45.0f, GetColor(temp, 125.0f), 3);
    screen.draw(X - 27, Y - 11, TempGaugeBoxBitmap);
    DrawValueText(screen, X + 23, Y - 5, temp, 3, false);
}

void Graphics::DrawGearboxTempGauge(Display::Screen& screen, float temp) {
    constexpr int X = 260, Y = 198;
    temp = Utility::Clamp(temp, -999.0f, 999.9f);

    float angle = Utility::Map(temp, 45.0f, 135.0f, 180.0f, 360.0f, true);
    screen.draw(X - 50, Y - 48, TempGaugeBitmap);
    DrawDirectedLine(screen, X, Y, angle, 20.0f, 45.0f, GetColor(temp, 112.5f), 3);
    screen.draw(X - 27, Y - 11, TempGaugeBoxBitmap);
    DrawValueText(screen, X + 23, Y - 5, temp, 3, false);
}

} // namespace evms
