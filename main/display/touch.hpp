#pragma once

#include <cstdint>
#include <string>

#include "display/types.hpp"
#include "drivers/gpio_pin.hpp"
#include "drivers/spi_bus.hpp"

namespace evms {

namespace Display {
    class Touch : private Drivers::SpiDevice {
    private:
        std::string m_logTag;
        Drivers::GpioPin m_irqPin;
        
    public:
        Touch(const Drivers::SpiBus& spiBus, gpio_num_t csPin, gpio_num_t irqPin);

        ~Touch();

    private:
        uint16_t command(uint8_t commandCode);

    public:
        bool isTouched();

        // { -1, -1 } if not touched
        Position getTouchPosition();

        // 0 if not touched
        int getTouchPressure();

        // In Celcius, ~5-10C error
        float getControllerTemp();
    };
}

} // namespace evms
