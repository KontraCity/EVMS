#include "touch.hpp"

#include <utility>

namespace evms {

Display::Touch::Touch(const Drivers::SpiBus& spiBus, gpio_num_t csPin, gpio_num_t irqPin)
    : SpiDevice(spiBus.newDevice("XPT2046", csPin, 2'000'000, true))
    , m_irqPin("IRQ", irqPin, GPIO_MODE_INPUT) {
}

Display::Touch::Touch(Touch&& other) noexcept
    : SpiDevice(std::move(other))
    , m_irqPin(std::move(other.m_irqPin))
{}

Display::Touch& Display::Touch::operator=(Touch&& other) noexcept {
    if (&other != this) {
        SpiDevice::operator=(std::move(other));
        m_irqPin = std::move(other.m_irqPin);
    }
    return *this;
}

uint16_t Display::Touch::getValue(uint8_t valueCode) const {
    constexpr uint8_t PowerMode = 0b00;
    valueCode &= ~0b11; 
    valueCode |= PowerMode;

    std::vector<uint8_t> response = transfer({ valueCode, 0x00, 0x00 }, 3);
    return ((response[1] << 8) | response[2]) >> 3;
}

bool Display::Touch::isTouched() const {
    return !m_irqPin.read();
}

Display::Position Display::Touch::getTouchPosition() const {
    if (!isTouched())
        return { -1, -1 };

    uint16_t x = getValue(0b1'101'00'00);
    uint16_t y = getValue(0b1'001'00'00);
    return { x, y };
}

int Display::Touch::getTouchPressure() const {
    if (!isTouched())
        return 0;

    uint16_t x = getValue(0b1'101'00'00);
    uint16_t z1 = getValue(0b1'011'00'00);
    uint16_t z2 = getValue(0b1'100'00'00);

    if (z1 >= z2)
        return 0;

    int pressure = x;
    pressure *= (z2 - z1);
    pressure /= z1;
    return pressure;
}

float Display::Touch::getControllerTemp() const {
    uint16_t t0 = getValue(0b1'000'01'00);
    uint16_t t1 = getValue(0b1'111'01'00);
    return (t1 - t0) * 0.125f;
}

} // namespace evms
