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

    public:
        SpiBus(const char* logName, spi_host_device_t host, gpio_num_t sckPin, gpio_num_t mosiPin, gpio_num_t misoPin);

        SpiBus(const SpiBus& other) = delete;

        SpiBus(SpiBus&& other) noexcept;

        ~SpiBus();

    public:
        SpiBus& operator=(const SpiBus& other) = delete;

        SpiBus& operator=(SpiBus&& other) noexcept;

    public:
        SpiDevice newDevice(const char* logName, gpio_num_t csPin, int frequency, bool fullDuplex = true) const;
    };
}

} // namespace evms
