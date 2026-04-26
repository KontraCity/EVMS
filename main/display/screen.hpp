#pragma once

#include <cstdint>
#include <cstring>
#include <vector>
#include <algorithm>

#include "display/types.hpp"
#include "drivers/gpio_pin.hpp"
#include "drivers/spi_bus.hpp"

namespace evms {

namespace Display {
    class Screen : private Drivers::SpiDevice {
    public:
        // A typical ST7796S screen is 480x320 pixels
        static constexpr int ScreenWidth = 480;
        static constexpr int ScreenHeight = 320;
        static constexpr Size ScreenSize = { ScreenWidth, ScreenHeight };

    private:
        Drivers::GpioPin m_resetPin;
        Drivers::GpioPin m_dcPin;

    public:
        Screen(const Drivers::SpiBus& spiBus, gpio_num_t csPin, gpio_num_t resetPin, gpio_num_t dcPin);

        Screen(const Screen& other) = delete;

        Screen(Screen&& other) noexcept;

        ~Screen() = default;

    public:
        Screen& operator=(const Screen& other) = delete;

        Screen& operator=(Screen&& other) noexcept;

    private:
        void reset();

        std::vector<uint8_t> command(uint8_t commandCode, const std::vector<uint8_t>& parameters = {}, size_t responseLength = 0);

    public:
        void clear();

        void clear(int x, int y, Size size);

        template <typename Map>
        void render(int x, int y, const Map& map);

    public:
        inline int width() const {
            return ScreenWidth;
        }

        inline int height() const {
            return ScreenHeight;
        }

        inline Size size() const {
            return ScreenSize;
        }
    };
}

} // namespace evms

#include "screen.inl"
