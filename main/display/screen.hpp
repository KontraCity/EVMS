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
        // A typical ILI9341 screen is 320x240 pixels
        static constexpr int ScreenWidth = 320;
        static constexpr int ScreenHeight = 240;
        static constexpr Size ScreenSize = { 320, 240 };

    private:
        static PixelMap<ScreenSize> s_framebuffer;

    private:
        Drivers::GpioPin m_resetPin;
        Drivers::GpioPin m_dcPin;

        // Render region variables
        int m_xStart = -1, m_xEnd = -1;
        int m_yStart = -1, m_yEnd = -1;

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

        bool framebufferChanged() const;

    public:
        void markChangedRegion(int x, int y, int width, int height);

        void clear();

        void clear(int x, int y, Size size);

        template <typename Map>
        void draw(int x, int y, const Map& map);

        void setPixelUnmarked(int x, int y, uint16_t pixel);

        void blendPixelUnmarked(int x, int y, uint16_t pixel, uint8_t alpha);

        void render();

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

        inline const PixelMap<ScreenSize>& framebuffer() const {
            return s_framebuffer;
        }
    };
}

} // namespace evms

#include "screen.inl"
