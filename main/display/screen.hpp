#pragma once

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include <algorithm>

#include "display/types.hpp"
#include "drivers/gpio_pin.hpp"
#include "drivers/spi_bus.hpp"

namespace evms {

namespace Display {
    class Screen : private Drivers::SpiDevice {
    public:
        // A typical ILI9341 screen is 240x320 pixels
        static constexpr Dimensions2D Dimensions = { 240, 320 };

    private:
        static PixelMap<Dimensions> s_framebuffer;

    private:
        std::string m_logTag;
        Drivers::GpioPin m_resetPin;
        Drivers::GpioPin m_dcPin;

        // Render region variables
        int m_xStart = -1, m_xEnd = -1;
        int m_yStart = -1, m_yEnd = -1;

    public:
        Screen(const Drivers::SpiBus& spiBus, gpio_num_t csPin, gpio_num_t resetPin, gpio_num_t dcPin);

        ~Screen();

    private:
        void reset();

        std::vector<uint8_t> command(uint8_t commandCode, const std::vector<uint8_t>& parameters = {}, size_t responseLength = 0);

        bool framebufferChanged() const;

        void markChangedRegion(int xStart, int xEnd, int yStart, int yEnd);

    public:
        void clear();

        void clear(int x, int y, Dimensions2D dimensions);

        template <typename Map>
        void draw(int x, int y, const Map& map);

        void render();

    public:
        inline const PixelMap<Dimensions>& framebuffer() const {
            return s_framebuffer;
        }
    };
}

} // namespace evms

#include "screen.inl"
