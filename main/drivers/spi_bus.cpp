#include "spi_bus.hpp"

#include <utility>

#include <esp_log.h>

namespace evms {

static std::string MakeLogTag(const std::string& logName, spi_host_device_t host) {
    std::string hostStr = Drivers::SpiBus::HostToString(host);
    return logName + " SpiBus [" + hostStr + "]";
}

const char* Drivers::SpiBus::HostToString(spi_host_device_t host) {
    switch (host) {
        case SPI1_HOST: return "ISPI";
        case SPI2_HOST: return "HSPI";
        case SPI3_HOST: return "VSPI";
        default:        return "<unknown>";
    } 
}

Drivers::SpiBus::SpiBus(const char* logName, spi_host_device_t host, gpio_num_t sckPin, gpio_num_t mosiPin, gpio_num_t misoPin)
    : m_logTag(MakeLogTag(logName, host))
    , m_host(host) {
    if (m_host == SPI1_HOST) {
        ESP_LOGE(m_logTag.c_str(), "SPI1_HOST host is reserved for flash and cannot be used");
        ESP_ERROR_CHECK(ESP_ERR_INVALID_ARG);
    }

    spi_bus_config_t busConfig = {};
    busConfig.sclk_io_num = sckPin;
    busConfig.mosi_io_num = mosiPin;
    busConfig.miso_io_num = misoPin;
    busConfig.quadwp_io_num = -1;
    busConfig.quadhd_io_num = -1;
    busConfig.max_transfer_sz = 160000;
    ESP_ERROR_CHECK(spi_bus_initialize(m_host, &busConfig, SPI_DMA_CH_AUTO));
    ESP_LOGI(m_logTag.c_str(), "Initialized with pins: \"SCK\" - %d, \"MOSI\" - %d, \"MISO\" - %d", sckPin, mosiPin, misoPin);
}

Drivers::SpiBus::SpiBus(SpiBus&& other) noexcept
    : m_logTag(std::move(other.m_logTag))
    , m_host(std::exchange(other.m_host, SPI_HOST_MAX))
{}

Drivers::SpiBus::~SpiBus() {
    if (m_host != SPI_HOST_MAX) {
        ESP_ERROR_CHECK(spi_bus_free(m_host));
        ESP_LOGI(m_logTag.c_str(), "Deinitialized");
    }
}

Drivers::SpiBus& Drivers::SpiBus::operator=(SpiBus&& other) noexcept {
    if (&other != this) {
        m_logTag = std::move(other.m_logTag);
        m_host = std::exchange(other.m_host, SPI_HOST_MAX);
    }
    return *this;
}

Drivers::SpiDevice Drivers::SpiBus::newDevice(const char* logName, gpio_num_t csPin, int frequency, bool fullDuplex) const {
    return { logName, m_host, csPin, frequency, fullDuplex };
}

} // namespace evms
