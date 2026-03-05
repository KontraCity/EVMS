#include "touch.hpp"

#include <esp_log.h>

namespace evms {

static std::string MakeLogTag() {
    return "Touch";
}

Display::Touch::Touch(const Drivers::SpiBus& spiBus, gpio_num_t csPin, gpio_num_t irqPin)
    : SpiDevice(spiBus.newDevice(csPin, 2'000'000, true))
    , m_logTag(MakeLogTag())
    , m_irqPin(irqPin, GPIO_MODE_INPUT) {
    ESP_LOGI(m_logTag.c_str(), "Initialized");
}

Display::Touch::~Touch() {
    ESP_LOGI(m_logTag.c_str(), "Deinitialized");
}

uint16_t Display::Touch::command(uint8_t commandCode) {
    constexpr uint8_t PowerMode = 0b00;
    commandCode &= ~0b11; 
    commandCode |= PowerMode;

    std::vector<uint8_t> response = transfer({ commandCode, 0x00, 0x00 }, 3);
    return ((response[1] << 8) | response[2]) >> 3;
}

bool Display::Touch::isTouched() {
    return !m_irqPin.read();
}

Display::Position Display::Touch::getTouchPosition() {
    if (!isTouched())
        return { -1, -1 };

    uint16_t x = command(0b1'101'00'00);
    uint16_t y = command(0b1'001'00'00);
    return { x, y };
}

int Display::Touch::getTouchPressure() {
    if (!isTouched())
        return 0;

    uint16_t x = command(0b1'101'00'00);
    uint16_t z1 = command(0b1'011'00'00);
    uint16_t z2 = command(0b1'100'00'00);

    if (z1 >= z2)
        return 0;

    int pressure = x;
    pressure *= (z2 - z1);
    pressure /= z1;
    return pressure;
}

float Display::Touch::getControllerTemp() {
    uint16_t t0 = command(0b1'000'01'00);
    uint16_t t1 = command(0b1'111'01'00);
    return (t1 - t0) * 0.125f;
}

} // namespace evms
