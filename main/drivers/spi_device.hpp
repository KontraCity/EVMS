#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <driver/gpio.h>
#include <driver/spi_master.h>

namespace evms {

namespace Drivers {
    class SpiDevice {
    private:
        std::string m_logTag;
        spi_device_handle_t m_handle;
        gpio_num_t m_csPin;

    public:
        SpiDevice(spi_host_device_t host, gpio_num_t csPin, int frequency, bool fullDuplex = true);

        ~SpiDevice();

    public:
        std::vector<uint8_t> transfer(const std::vector<uint8_t>& data, size_t responseLength);

        void send(const std::vector<uint8_t>& data);

        void send(const uint8_t* data, size_t length);

        std::vector<uint8_t> receive(size_t length);
    };
}

} // namespace evms
