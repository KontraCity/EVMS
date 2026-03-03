#include <iostream>
#include <thread>

#include "display/screen.hpp"
#include "drivers/pwm_led.hpp"
#include "drivers/spi_bus.hpp"
#include "utility/random.hpp"
#include "utility/time.hpp"
#include "dvd_logo.hpp"
using namespace evms;

/*
*   Connection to the 2.4" TFT display:
*   Screen      ESP32     Wire
*   VCC         3.3       (Red)
*   GND         GND       (Dark)
*   CS           15       (Green)
*   RESET         4       (Gray)
*   DC            2       (Green)
*   MOSI         23       (Orange)
*   SCK          18       (Orange)
*   LED          22       (Blue)
*   MISO         19       (Orange)
*/

static void EnableBacklightSmoothly(Drivers::PwmLed& backlight) {
    for (float percentage = 0.0f; percentage <= 100.0f; percentage += 3.3f) {
        backlight.setDutyPercent(percentage);
        Utility::Sleep(0.1f);
    }
}

extern "C" void app_main() {
    Drivers::SpiBus spiBus(SPI2_HOST, GPIO_NUM_18, GPIO_NUM_23, GPIO_NUM_19);
    Display::Screen display(spiBus, GPIO_NUM_15, GPIO_NUM_4, GPIO_NUM_2);
    Drivers::PwmLed backlight(LEDC_CHANNEL_0, GPIO_NUM_22);
    std::thread backlightThread;

    constexpr int Speed = 1;
    int xSpeed = Utility::Random(0, 1) ? Speed : -Speed;
    int ySpeed = Utility::Random(0, 1) ? Speed : -Speed;
    int borderHits = 0, cornerHits = 0;
    int x = Utility::Random(0, Display::Screen::Dimensions.width - DvdLogo.dimensions().width);
    int y = Utility::Random(0, Display::Screen::Dimensions.height - DvdLogo.dimensions().height);

    while (true) {
        bool horizontalBorderHit = false;
        if (x <= 0) {
            xSpeed = Speed;
            horizontalBorderHit = true;
        }
        else if (x + DvdLogo.dimensions().width >= (Display::Screen::Dimensions.width - 1)) {
            xSpeed = -Speed;
            horizontalBorderHit = true;
        }

        bool verticalBorderHit = false;
        if (y <= 0) {
            ySpeed = Speed;
            verticalBorderHit = true;
        }
        else if (y + DvdLogo.dimensions().height >= (Display::Screen::Dimensions.height - 1)) {
            ySpeed = -Speed;
            verticalBorderHit = true;
        }

        if (horizontalBorderHit && verticalBorderHit)
            ++cornerHits;
        else if (horizontalBorderHit || verticalBorderHit)
            ++borderHits;

        if (horizontalBorderHit || verticalBorderHit) {
            std::cout << "Time: " << Utility::TimeSeconds() << "s, ";
            std::cout << "border hits: " << borderHits << ", ";
            std::cout << "corner hits: " << cornerHits << '\n';
        }

        display.clear(x, y, DvdLogo.dimensions());
        x += xSpeed; y += ySpeed;
        display.draw(x, y, DvdLogo);
        display.render();
        Utility::Sleep(0.01);

        static bool s_firstTime = true;
        if (s_firstTime) {
            backlightThread = std::thread(&EnableBacklightSmoothly, std::ref(backlight));
            s_firstTime = false;
        }
    }
}
