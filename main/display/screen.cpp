#include "screen.hpp"

#include <utility>

#include "utility/pixel.hpp"
#include "utility/time.hpp"

namespace evms {

Display::Screen::Screen(const Drivers::SpiBus& spiBus, gpio_num_t csPin, gpio_num_t resetPin, gpio_num_t dcPin)
    : SpiDevice(spiBus.newDevice("ST7796S", csPin, 80'000'000, false))
    , m_resetPin("RESET", resetPin, GPIO_MODE_OUTPUT)
    , m_dcPin("DC", dcPin, GPIO_MODE_OUTPUT) {
    // Reset and get out of sleep
    reset();
    command(0x11);
    Utility::Sleep(0.12f);

    // Configure
    command(0x3A, { 0b0'101'0'101 });   // Interface pixel format
    command(0x36, { 0b111'000'00 });    // Memory data access controlv
    command(0x29);                      // Display on

    // Clear garbage in GRAM
    clear();
}

Display::Screen::Screen(Screen&& other) noexcept
    : SpiDevice(std::move(other))
    , m_resetPin(std::move(other.m_resetPin))
    , m_dcPin(std::move(other.m_dcPin))
{}

Display::Screen& Display::Screen::operator=(Screen&& other) noexcept {
    if (&other != this) {
        SpiDevice::operator=(std::move(other));
        m_resetPin = std::move(other.m_resetPin);
        m_dcPin = std::move(other.m_dcPin);
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

void Display::Screen::clear() {
    clear(0, 0, ScreenSize);
}

void Display::Screen::clear(int x, int y, Size size) {
    if (x < 0) {
        int widthStart = -x;
        size.width -= widthStart;
        x = 0;
    }
    if (y < 0) {
        int heightStart = -y;
        size.height -= heightStart;
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

    // Column address set (X)
    int columnStart = x;
    int columnEnd = x + size.width - 1;
    command(0x2A, {
        static_cast<uint8_t>(columnStart >> 8),
        static_cast<uint8_t>(columnStart & 0xFF),
        static_cast<uint8_t>(columnEnd >> 8),
        static_cast<uint8_t>(columnEnd & 0xFF)
    });

    // Row address set (Y)
    int rowStart = y;
    int rowEnd = y + size.height - 1;
    command(0x2B, {
        static_cast<uint8_t>(rowStart >> 8),
        static_cast<uint8_t>(rowStart & 0xFF),
        static_cast<uint8_t>(rowEnd >> 8),
        static_cast<uint8_t>(rowEnd & 0xFF)
    });

    command(0x2C);
    m_dcPin.write(true);
    for (int row = 0; row < size.height; ++row) {
        static const PixelMap<{ ScreenWidth, 1 }> ClearLine = {}; // hacky - do something about it?
        send(reinterpret_cast<const uint8_t*>(ClearLine.data()), size.width * sizeof(uint16_t));
    }
}

} // namespace evms
