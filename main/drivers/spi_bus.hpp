#pragma once

#include <string>

#include <driver/gpio.h>
#include <driver/spi_master.h>

#include "drivers/spi_device.hpp"

namespace evms {

namespace Drivers {
    class SpiBus {
    public:
        static const char* HostToString(spi_host_device_t host);

    private:
        std::string m_logTag;
        spi_host_device_t m_host;
        gpio_num_t m_sckPin;
        gpio_num_t m_mosiPin;
        gpio_num_t m_misoPin;

    public:
        SpiBus(spi_host_device_t host, gpio_num_t sckPin, gpio_num_t mosiPin, gpio_num_t misoPin);

        ~SpiBus();

    public:
        SpiDevice newDevice(gpio_num_t csPin, int frequency, bool halfDuplex = false) const;
    };
}

} // namespace evms
