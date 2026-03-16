#pragma once

#include <string>

#include "display/screen.hpp"
#include "display/touch.hpp"
#include "drivers/gpio_pwm.hpp"
#include "drivers/spi_bus.hpp"
#include "fonts/ecam_font.hpp"

namespace evms {

namespace App {
    class Ui {
    private:
        using Screen = Display::Screen;
        using Glyph = Fonts::EcamFontGlyph;
    
    public:
        struct Config {
            gpio_num_t csPin;
            gpio_num_t resetPin;
            gpio_num_t dcPin;
            gpio_num_t mosiPin;
            gpio_num_t sckPin;
            gpio_num_t ledPin;
            gpio_num_t misoPin;
            gpio_num_t tcsPin;
        };

    private:
        std::string m_logTag;
        Drivers::SpiBus m_spiBus;
        Drivers::GpioPwm m_backlight;
        Display::Screen m_screen;
        Display::Touch m_touch;

    public:
        Ui(const Config& config);

        Ui(const Ui& other) = delete;

        Ui(Ui&& other) noexcept;

        ~Ui();

    public:
        Ui& operator=(const Ui& other) = delete;

        Ui& operator=(Ui&& other) noexcept;

    private:
        void calibrateTouch();

        bool applyTouchCalibration(bool overwrite, Display::Touch::Calibration calibration = {});

        Display::Size getTextSize(const std::string& text);

        void printText(int x, int y, const std::string& text);

        void printCenteredText(const std::vector<std::string>& lines);

    public:
        void mainloop();
    };
}

} // namespace evms
