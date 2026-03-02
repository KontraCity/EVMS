#include "spi_device.hpp"

#include <esp_log.h>

#include "spi_bus.hpp"

namespace evms {

static std::string MakeLogTag(spi_host_device_t host, gpio_num_t csPin) {
    std::string hostStr = Drivers::SpiBus::HostToString(host);
    std::string csPinStr = std::to_string(static_cast<int>(csPin));
    return "SpiDevice [" + hostStr + ", CS_" + csPinStr + "]";
}

Drivers::SpiDevice::SpiDevice(spi_host_device_t host, gpio_num_t csPin, int frequency, bool halfDuplex)
    : m_logTag(MakeLogTag(host, csPin))
    , m_handle(0)
    , m_csPin(csPin) {
    spi_device_interface_config_t spiDeviceConfig = {};
    spiDeviceConfig.clock_speed_hz = frequency;
    spiDeviceConfig.mode = 0;
    spiDeviceConfig.spics_io_num = m_csPin;
    spiDeviceConfig.queue_size = 1;
    spiDeviceConfig.flags = halfDuplex ? SPI_DEVICE_HALFDUPLEX : 0;
    ESP_ERROR_CHECK(spi_bus_add_device(host, &spiDeviceConfig, &m_handle));
    ESP_LOGI(m_logTag.c_str(), "Initialized with frequency \"%d\"", frequency);
}

Drivers::SpiDevice::~SpiDevice() {
    ESP_ERROR_CHECK(spi_bus_remove_device(m_handle));
    ESP_LOGI(m_logTag.c_str(), "Deinitialized");
}

std::vector<uint8_t> Drivers::SpiDevice::transfer(const std::vector<uint8_t>& dataOut, int dataInSize) {
    std::vector<uint8_t> dataIn(dataInSize);
    spi_transaction_t transaction = {};
    transaction.length = dataOut.size() * 8;
    transaction.tx_buffer = dataOut.data();
    transaction.rxlength = dataIn.size() * 8;
    transaction.rx_buffer = dataIn.data();
    ESP_ERROR_CHECK(spi_device_transmit(m_handle, &transaction));
    return dataIn;
}

void Drivers::SpiDevice::send(const std::vector<uint8_t>& data) {
    spi_transaction_t transaction = {};
    transaction.length = data.size() * 8;
    transaction.tx_buffer = data.data();
    transaction.rxlength = 0;
    transaction.rx_buffer = nullptr;
    ESP_ERROR_CHECK(spi_device_transmit(m_handle, &transaction));
}

std::vector<uint8_t> Drivers::SpiDevice::receive(int size) {
    std::vector<uint8_t> buffer(size);
    spi_transaction_t transaction = {};
    transaction.length = 0;
    transaction.tx_buffer = nullptr;
    transaction.rxlength = buffer.size() * 8;
    transaction.rx_buffer = buffer.data();
    ESP_ERROR_CHECK(spi_device_transmit(m_handle, &transaction));
    return buffer;
}

} // namespace evms
