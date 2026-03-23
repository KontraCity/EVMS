#include "screen.hpp"

#include <utility>

#include "utility/pixel.hpp"
#include "utility/time.hpp"

namespace evms {

Display::PixelMap<Display::Screen::ScreenSize> Display::Screen::s_framebuffer = {};

Display::Screen::Screen(const Drivers::SpiBus& spiBus, gpio_num_t csPin, gpio_num_t resetPin, gpio_num_t dcPin)
    : SpiDevice(spiBus.newDevice("ILI9341", csPin, 42'000'000, false))
    , m_resetPin("RESET", resetPin, GPIO_MODE_OUTPUT)
    , m_dcPin("DC", dcPin, GPIO_MODE_OUTPUT) {
    // Reset and sleep out
    reset();
    command(0x11);
    Utility::Sleep(0.12f);

    // Configure
    command(0x29);                      // Display on
    command(0x3A, { 0b0'101'0'101 });   // Interface pixel format
    command(0x36, { 0b011'000'00 });    // Memory data access control
    
    // Clear garbage in GRAM
    clear();
    render();
}

Display::Screen::Screen(Screen&& other) noexcept
    : SpiDevice(std::move(other))
    , m_resetPin(std::move(other.m_resetPin))
    , m_dcPin(std::move(other.m_dcPin))
    , m_xStart(std::exchange(other.m_xStart, -1))
    , m_xEnd(std::exchange(other.m_xEnd, -1))
    , m_yStart(std::exchange(other.m_yStart, -1))
    , m_yEnd(std::exchange(other.m_yEnd, -1))
{}

Display::Screen& Display::Screen::operator=(Screen&& other) noexcept {
    if (&other != this) {
        SpiDevice::operator=(std::move(other));
        m_resetPin = std::move(other.m_resetPin);
        m_dcPin = std::move(other.m_dcPin);
        m_xStart = std::exchange(other.m_xStart, -1);
        m_xEnd = std::exchange(other.m_xEnd, -1);
        m_yStart = std::exchange(other.m_yStart, -1);
        m_yEnd = std::exchange(other.m_yEnd, -1);
    }
    return *this;
}

void Display::Screen::reset() {
    m_resetPin.write(false);
    Utility::Sleep(0.05);
    m_resetPin.write(true);
    Utility::Sleep(0.12);
}

std::vector<uint8_t> Display::Screen::command(uint8_t commandCode, const std::vector<uint8_t>& parameters, size_t responseLength) {
    m_dcPin.write(false);
    send(&commandCode, 1);

    if (!parameters.empty()) {
        m_dcPin.write(true);
        send(parameters);
    }

    std::vector<uint8_t> response;
    if (responseLength) {
        m_dcPin.write(true);
        response = receive(responseLength);
    }
    return response;
}

bool Display::Screen::framebufferChanged() const {
    return (
        m_xStart != -1 &&
        m_xEnd   != -1 &&
        m_yStart != -1 &&
        m_yEnd   != -1
    );
}

void Display::Screen::markChangedRegion(int x, int y, int width, int height) {
    if (x < 0) {
        width += x;
        x = 0;
    }
    if (y < 0) {
        height += y;
        y = 0;
    }

    if (x + width > ScreenWidth)
        width = ScreenWidth - x;
    if (y + height > ScreenHeight)
        height = ScreenHeight - y;

    if (width <= 0 || height <= 0) {
        // Changed region is empty or out of screen bounds!
        return;
    }

    if (!framebufferChanged()) {
        m_xStart = x;
        m_xEnd = x + width - 1;
        m_yStart = y;
        m_yEnd = y + height - 1;
    }
    else {
        m_xStart = std::min(m_xStart, x);
        m_xEnd = std::max(m_xEnd, x + width - 1);
        m_yStart = std::min(m_yStart, y);
        m_yEnd = std::max(m_yEnd, y + height - 1);
    }
}

void Display::Screen::clear() {
    clear(0, 0, ScreenSize);
}

void Display::Screen::clear(int x, int y, Size size) {
    if (x < 0) {
        size.width += x;
        x = 0;
    }
    if (y < 0) {
        size.height += y;
        y = 0;
    }

    if (x + size.width > ScreenWidth)
        size.width = ScreenWidth - x;
    if (y + size.height > ScreenHeight)
        size.height = ScreenHeight - y;

    if (size.width <= 0 || size.height <= 0) {
        // Clear region is empty or out of screen bounds!
        return;
    }

    for (int row = 0; row < size.height; ++row) {
        uint16_t* regionRow = s_framebuffer.data() + ((y + row) * ScreenWidth) + x;
        std::memset(regionRow, 0, size.width * sizeof(uint16_t));
    }
    markChangedRegion(x, y, size.width, size.height);
}

void Display::Screen::setPixelUnmarked(int x, int y, uint16_t pixel) {
    if (x < 0 || y < 0 || x >= ScreenWidth || y >= ScreenHeight) {
        // Pixel is out of screen bounds!
        return;
    }

    s_framebuffer[y * ScreenWidth + x] = __builtin_bswap16(pixel);
}

void Display::Screen::blendPixelUnmarked(int x, int y, uint16_t pixel, uint8_t alpha) {
    if (x < 0 || y < 0 || x >= ScreenWidth || y >= ScreenHeight) {
        // Pixel is out of screen bounds!
        return;
    }

    uint16_t& framebufferPixel = s_framebuffer[y * ScreenWidth + x];
    uint16_t blended = Utility::Blend565RGB(__builtin_bswap16(framebufferPixel), pixel, alpha);
    framebufferPixel = __builtin_bswap16(blended);
}

void Display::Screen::render() {
    // Check if framebuffer and GRAM match
    if (!framebufferChanged())
        return;

    // Column address set (X)
    command(0x2A, {
        static_cast<uint8_t>(m_xStart >> 8), static_cast<uint8_t>(m_xStart),
        static_cast<uint8_t>(m_xEnd >> 8), static_cast<uint8_t>(m_xEnd)
    });

    // Row address set (Y)
    command(0x2B, {
        static_cast<uint8_t>(m_yStart >> 8), static_cast<uint8_t>(m_yStart),
        static_cast<uint8_t>(m_yEnd >> 8), static_cast<uint8_t>(m_yEnd)
    });
    
    int regionWidth = m_xEnd - m_xStart + 1;
    int regionHeight = m_yEnd - m_yStart + 1;

    float start = Utility::TimeSeconds();
    command(0x2C);
    m_dcPin.write(true);
    for (int row = 0; row < regionHeight; ++row) {
        const uint16_t* regionRow = s_framebuffer.data() + ((m_yStart + row) * ScreenWidth) + m_xStart;
        send(reinterpret_cast<const uint8_t*>(regionRow), regionWidth * sizeof(uint16_t));
    }

    // Framebuffer and GRAM match now
    m_xStart = -1; m_xEnd = -1;
    m_yStart = -1; m_yEnd = -1;
}

} // namespace evms
