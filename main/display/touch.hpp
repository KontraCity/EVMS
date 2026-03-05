#pragma once

#include <cstdint>

#include "display/types.hpp"
#include "drivers/gpio_pin.hpp"
#include "drivers/spi_bus.hpp"

namespace evms {

namespace Display {
    class Touch : private Drivers::SpiDevice {
    private:
        Drivers::GpioPin m_irqPin;
        
    public:
        Touch(const Drivers::SpiBus& spiBus, gpio_num_t csPin, gpio_num_t irqPin);

        Touch(const Touch& other) = delete;

        Touch(Touch&& other) noexcept;

        ~Touch() = default;

    public:
        Touch& operator=(const Touch& other) = delete;

        Touch& operator=(Touch&& other) noexcept;

    private:
        uint16_t getValue(uint8_t valueCode) const;

    public:
        bool isTouched() const;

        // { -1, -1 } if not touched
        Position getTouchPosition() const;

        // 0 if not touched
        int getTouchPressure() const;

        // In Celcius, ~5-10C error
        float getControllerTemp() const;
    };
}

} // namespace evms
