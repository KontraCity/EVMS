#include <iostream>
#include <thread>
#include <algorithm>

#include "display/screen.hpp"
#include "display/touch.hpp"
#include "drivers/pwm_led.hpp"
#include "drivers/spi_bus.hpp"
#include "utility/random.hpp"
#include "utility/math.hpp"
#include "utility/time.hpp"
#include "bitmaps.hpp"
using namespace evms;

/*
*   Connection to the 2.4" TFT display:
*   Screen      ESP32
*   VCC         3.3
*   GND         GND
*   CS           15
*   RESET         4
*   DC            2
*   MOSI         23
*   SCK          18
*   LED          22
*   MISO         19
*   T_CS         21
*   T_IRQ        5
*/

static void EnableBacklightSmoothly(Drivers::PwmLed& backlight) {
    for (float percentage = 0.0f; percentage <= 100.0f; percentage += 3.3f) {
        backlight.setDutyPercent(percentage);
        Utility::Sleep(0.1f);
    }
}

static bool AllNotZero(const uint16_t* base, int length, int stride) {
    for (int index = 0; index < length; ++index)
        if (base[index * stride] != 0)
            return false;
    return true;
}

static bool ColumnNotZero(Display::Screen& screen, int x, int y, int height) {
    constexpr int ScreenHeight = Display::Screen::Dimensions.height;
    if (y + height >= ScreenHeight)
        height -= (y + height) - ScreenHeight;

    constexpr int ScreenWidth = Display::Screen::Dimensions.width;
    const auto& framebuffer = screen.framebuffer();
    const uint16_t* begin = framebuffer.data() + ((y * ScreenWidth) + x);
    return !AllNotZero(begin, height, ScreenWidth);
}

static bool RowNotZero(Display::Screen& screen, int x, int y, int width) {
    constexpr int ScreenWidth = Display::Screen::Dimensions.width;
    if (x + width >= ScreenWidth)
        width -= (x + width) - ScreenWidth;

    const auto& framebuffer = screen.framebuffer();
    const uint16_t* begin = framebuffer.data() + ((y * ScreenWidth) + x);
    return !AllNotZero(begin, width, 1);
}

static Display::Position GetPosition(Display::Touch& touch) {
    Display::Position pos = touch.getTouchPosition();
    if (pos.x < 0 || pos.y < 0)
        return pos;

    // Manually calibrated for now
    pos.x = static_cast<int>(std::round(Utility::ConvertRange(
        static_cast<float>(pos.x),
        300.0f, 3900.0f,
        0.0f, 320.0f
    )));
    pos.y = static_cast<int>(std::round(Utility::ConvertRange(
        static_cast<float>(pos.y),
        250.0f, 3900.0f,
        0.0f, 240.0f
    )));
    return pos;
}

extern "C" void app_main() {
    Drivers::SpiBus spiBus(SPI2_HOST, GPIO_NUM_18, GPIO_NUM_23, GPIO_NUM_19);
    Display::Screen display(spiBus, GPIO_NUM_15, GPIO_NUM_4, GPIO_NUM_2);
    Display::Touch touch(spiBus, GPIO_NUM_21, GPIO_NUM_5);
    Drivers::PwmLed backlight(LEDC_CHANNEL_0, GPIO_NUM_22);
    std::thread backlightThread;

    constexpr Display::Dimensions2D ScreenDims = Display::Screen::Dimensions;
    constexpr Display::Dimensions2D LogoDims = Bitmaps::DvdLogo.dimensions();
    constexpr int Speed = 1;
    int xSpeed = Utility::Random(0, 1) ? Speed : -Speed;
    int ySpeed = Utility::Random(0, 1) ? Speed : -Speed;
    int canvasHits = 0, borderHits = 0, cornerHits = 0;
    int x = Utility::Random(0, ScreenDims.width - LogoDims.width);
    int y = Utility::Random(0, ScreenDims.height - LogoDims.height);

    while (true) {
        Display::Position pos = GetPosition(touch);
        if (pos.x >= 0 && pos.y >= 0) {
            display.draw(pos.y - 1, pos.x - 1, Bitmaps::Dot);
            display.render();
        }

        bool horizontalBorderHit = false;
        if (x <= 0) {
            xSpeed = Speed;
            horizontalBorderHit = true;
        }
        else if (x + LogoDims.width >= (ScreenDims.width - 1)) {
            xSpeed = -Speed;
            horizontalBorderHit = true;
        }
        
        bool xCanvasHit = false;
        if (x >= 1 && ColumnNotZero(display, x - 1, y, LogoDims.height)) {
            xSpeed = Speed;
            xCanvasHit = true;
        }
        else if (x + LogoDims.width < ScreenDims.width && ColumnNotZero(display, x + 1 + LogoDims.width, y, LogoDims.height)) {
            xSpeed = -Speed;
            xCanvasHit = true;
        }

        bool verticalBorderHit = false;
        if (y <= 0) {
            ySpeed = Speed;
            verticalBorderHit = true;
        }
        else if (y + LogoDims.height >= (ScreenDims.height - 1)) {
            ySpeed = -Speed;
            verticalBorderHit = true;
        }

        bool yCanvasHit = false;
        if (y >= 1 && RowNotZero(display, x, y - 1, LogoDims.width)) {
            ySpeed = Speed;
            yCanvasHit = true;
        }
        else if (y + LogoDims.height < ScreenDims.height && RowNotZero(display, x, y + 1 + LogoDims.height, LogoDims.width)) {
            ySpeed = -Speed;
            yCanvasHit = true;
        }

        if (horizontalBorderHit && verticalBorderHit)
            ++cornerHits;
        else if (horizontalBorderHit || verticalBorderHit)
            ++borderHits;
        else if (xCanvasHit || yCanvasHit)
            ++canvasHits;

        if (horizontalBorderHit || verticalBorderHit) {
            std::cout << "Time: " << Utility::TimeSeconds() << "s, ";
            std::cout << "canvas hits: " << canvasHits << ", ";
            std::cout << "border hits: " << borderHits << ", ";
            std::cout << "corner hits: " << cornerHits << '\n';
        }

        display.clear(x, y, LogoDims);
        x += xSpeed;
        y += ySpeed;
        display.draw(x, y, Bitmaps::DvdLogo);
        display.render();
        Utility::Sleep(0.01);

        static bool s_firstTime = true;
        if (s_firstTime) {
            backlightThread = std::thread(&EnableBacklightSmoothly, std::ref(backlight));
            s_firstTime = false;
        }
    }
}
