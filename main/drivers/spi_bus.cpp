#include "spi_bus.hpp"

#include <esp_log.h>

namespace evms {

static std::string MakeLogTag(spi_host_device_t host) {
    std::string hostStr = Drivers::SpiBus::HostToString(host);
    return "SpiBus [" + hostStr + "]";
}

const char* Drivers::SpiBus::HostToString(spi_host_device_t host) {
    switch (host) {
        case SPI1_HOST: return "ISPI";
        case SPI2_HOST: return "HSPI";
        case SPI3_HOST: return "VSPI";
        default:        return "<unknown>";
    } 
}

Drivers::SpiBus::SpiBus(spi_host_device_t host, gpio_num_t sckPin, gpio_num_t mosiPin, gpio_num_t misoPin)
    : m_logTag(MakeLogTag(host))
    , m_host(host)
    , m_sckPin(sckPin) 
    , m_mosiPin(mosiPin) 
    , m_misoPin(misoPin) {
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
    ESP_LOGI(m_logTag.c_str(), "Initialized with pins: \"SCK\" - %d, \"MOSI\" - %d, \"MISO\" - %d", m_sckPin, m_mosiPin, m_misoPin);
}

Drivers::SpiBus::~SpiBus() {
    ESP_ERROR_CHECK(spi_bus_free(m_host));
    ESP_LOGI(m_logTag.c_str(), "Deinitialized");
}

Drivers::SpiDevice Drivers::SpiBus::newDevice(gpio_num_t csPin, int frequency, bool halfDuplex) const {
    return { m_host, csPin, frequency, halfDuplex };
}

} // namespace evms
