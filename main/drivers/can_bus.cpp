#include "can_bus.hpp"

#include <utility>
#include <algorithm>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <esp_log.h>

namespace evms {

Drivers::CanBus::CanBus(const std::string& logName, gpio_num_t txPin, gpio_num_t rxPin, twai_mode_t mode)
    : m_logTag(logName + " CanBus") {
    twai_general_config_t generalConfig = TWAI_GENERAL_CONFIG_DEFAULT(txPin, rxPin, TWAI_MODE_NORMAL);
    twai_timing_config_t timingConfig = TWAI_TIMING_CONFIG_500KBITS();
    twai_filter_config_t filterConfig = TWAI_FILTER_CONFIG_ACCEPT_ALL();
    ESP_ERROR_CHECK(twai_driver_install(&generalConfig, &timingConfig, &filterConfig));
    ESP_ERROR_CHECK(twai_start());
    ESP_LOGI(m_logTag.c_str(), "Initialized with pins: \"RX\" - %d, \"TX\" - %d", rxPin, txPin);
}

Drivers::CanBus::CanBus(CanBus&& other) noexcept
    : m_logTag(std::move(other.m_logTag))
    , m_initialized(std::exchange(other.m_initialized, false))
{}

Drivers::CanBus::~CanBus() {
    if (m_initialized) {
        twai_stop();
        twai_driver_uninstall();
        ESP_LOGI(m_logTag.c_str(), "Deinitialized");
    }
}

Drivers::CanBus& Drivers::CanBus::operator=(CanBus&& other) noexcept {
    if (&other != this) {
        m_logTag = std::move(other.m_logTag);
        m_initialized = std::exchange(other.m_initialized, false);
    }
    return *this;
}

Drivers::CanBus::Message Drivers::CanBus::receive() const {
    while (true) {
        twai_message_t message;
        if (twai_receive(&message, pdMS_TO_TICKS(1000)) != ESP_OK)
            continue;

        Message result = {
            .id = message.identifier,
            .length = message.data_length_code,
            .data = {}
        };
        std::copy(
            message.data,
            message.data + message.data_length_code,
            result.data
        );
        return result;
    }
}

} // namespace evms
