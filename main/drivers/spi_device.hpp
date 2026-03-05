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

    public:
        SpiDevice(const char* logName, spi_host_device_t host, gpio_num_t csPin, int frequency, bool fullDuplex = true);

        SpiDevice(const SpiDevice& other) = delete;

        SpiDevice(SpiDevice&& other) noexcept;

        ~SpiDevice();

    public:
        SpiDevice& operator=(const SpiDevice& other) = delete;

        SpiDevice& operator=(SpiDevice&& other) noexcept;

    public:
        std::vector<uint8_t> transfer(const std::vector<uint8_t>& data, size_t responseLength) const;

        void send(const std::vector<uint8_t>& data) const;

        void send(const uint8_t* data, size_t length) const;

        std::vector<uint8_t> receive(size_t length) const;
    };
}

} // namespace evms
