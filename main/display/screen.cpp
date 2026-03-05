#include "screen.hpp"

#include <esp_log.h>

#include "utility/time.hpp"

namespace evms {

Display::PixelMap<Display::Screen::Dimensions> Display::Screen::s_framebuffer = {};

static std::string MakeLogTag() {
    return "Screen";
}

Display::Screen::Screen(const Drivers::SpiBus& spiBus, gpio_num_t csPin, gpio_num_t resetPin, gpio_num_t dcPin)
    : SpiDevice(spiBus.newDevice(csPin, 42'000'000, false))
    , m_logTag(MakeLogTag())
    , m_resetPin(resetPin, GPIO_MODE_OUTPUT)
    , m_dcPin(dcPin, GPIO_MODE_OUTPUT) {
    // Reset and sleep out
    reset();
    command(0x11);
    Utility::Sleep(0.12f);

    // Configure
    command(0x29);                      // Display on
    command(0x3A, { 0b0'101'0'101 });   // Interface pixel format
    command(0x36, { 0b000'000'00 });    // Memory data access control
    
    // Clear garbage in GRAM
    clear();
    render();

    ESP_LOGI(m_logTag.c_str(), "Initialized");
}

Display::Screen::~Screen() {
    ESP_LOGI(m_logTag.c_str(), "Deinitialized");
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
    clear(0, 0, Dimensions);
}

void Display::Screen::clear(int x, int y, Dimensions2D dimensions) {
    if (x < 0) {
        dimensions.width += x;
        x = 0;
    }
    if (y < 0) {
        dimensions.height += y;
        y = 0;
    }

    if (x + dimensions.width > Dimensions.width)
        dimensions.width = Dimensions.width - x;
    if (y + dimensions.height > Dimensions.height)
        dimensions.height = Dimensions.height - y;

    if (dimensions.width <= 0 || dimensions.height <= 0) {
        // Clear region is empty or out of screen bounds!
        return;
    }

    for (int row = 0; row < dimensions.height; ++row) {
        uint16_t* regionRow = s_framebuffer.data() + ((y + row) * Dimensions.width) + x;
        std::memset(regionRow, 0, dimensions.width * sizeof(uint16_t));
    }
    markChangedRegion(x, y, dimensions.width, dimensions.height);
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

    command(0x2C);
    m_dcPin.write(true);
    for (int row = 0; row < regionHeight; ++row) {
        const uint16_t* regionRow = s_framebuffer.data() + ((m_yStart + row) * Dimensions.width) + m_xStart;
        send(reinterpret_cast<const uint8_t*>(regionRow), regionWidth * sizeof(uint16_t));
    }

    // Framebuffer and GRAM match now
    m_xStart = -1; m_xEnd = -1;
    m_yStart = -1; m_yEnd = -1;
}

} // namespace evms
