#include "spi_device.hpp"

#include <esp_log.h>

#include "spi_bus.hpp"

namespace evms {

static std::string MakeLogTag(spi_host_device_t host, gpio_num_t csPin) {
    std::string hostStr = Drivers::SpiBus::HostToString(host);
    std::string csPinStr = std::to_string(static_cast<int>(csPin));
    return "SpiDevice [" + hostStr + ", CS_" + csPinStr + "]";
}

Drivers::SpiDevice::SpiDevice(spi_host_device_t host, gpio_num_t csPin, int frequency, bool fullDuplex)
    : m_logTag(MakeLogTag(host, csPin))
    , m_handle(0)
    , m_csPin(csPin) {
    spi_device_interface_config_t spiDeviceConfig = {};
    spiDeviceConfig.clock_speed_hz = frequency;
    spiDeviceConfig.mode = 0;
    spiDeviceConfig.spics_io_num = m_csPin;
    spiDeviceConfig.queue_size = 1;
    spiDeviceConfig.flags = fullDuplex ? 0 : SPI_DEVICE_HALFDUPLEX;
    ESP_ERROR_CHECK(spi_bus_add_device(host, &spiDeviceConfig, &m_handle));
    ESP_LOGI(m_logTag.c_str(), "Initialized with frequency \"%d\"", frequency);
}

Drivers::SpiDevice::~SpiDevice() {
    ESP_ERROR_CHECK(spi_bus_remove_device(m_handle));
    ESP_LOGI(m_logTag.c_str(), "Deinitialized");
}

std::vector<uint8_t> Drivers::SpiDevice::transfer(const std::vector<uint8_t>& data, size_t responseLength) {
    std::vector<uint8_t> response(responseLength);
    spi_transaction_t transaction = {};
    transaction.length = data.size() * 8;
    transaction.tx_buffer = data.data();
    transaction.rxlength = response.size() * 8;
    transaction.rx_buffer = response.data();
    ESP_ERROR_CHECK(spi_device_transmit(m_handle, &transaction));
    return response;
}

void Drivers::SpiDevice::send(const std::vector<uint8_t>& data) {
    spi_transaction_t transaction = {};
    transaction.length = data.size() * 8;
    transaction.tx_buffer = data.data();
    transaction.rxlength = 0;
    transaction.rx_buffer = nullptr;
    ESP_ERROR_CHECK(spi_device_transmit(m_handle, &transaction));
}

void Drivers::SpiDevice::send(const uint8_t* data, size_t length) {
    spi_transaction_t transaction = {};
    transaction.length = length * 8;
    transaction.tx_buffer = data;
    transaction.rxlength = 0;
    transaction.rx_buffer = nullptr;
    ESP_ERROR_CHECK(spi_device_transmit(m_handle, &transaction));
}

std::vector<uint8_t> Drivers::SpiDevice::receive(size_t length) {
    std::vector<uint8_t> buffer(length);
    spi_transaction_t transaction = {};
    transaction.length = 0;
    transaction.tx_buffer = nullptr;
    transaction.rxlength = buffer.size() * 8;
    transaction.rx_buffer = buffer.data();
    ESP_ERROR_CHECK(spi_device_transmit(m_handle, &transaction));
    return buffer;
}

} // namespace evms
