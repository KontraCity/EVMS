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

bool Drivers::CanBus::send(const Message& message) const {
    twai_message_t payload = {};
    payload.extd = 0;  // 11-bit ID
    payload.rtr = 0;   // Not a remote request
    payload.ss = 0;    // Not a single shot request
    payload.self = 0;  // Not a self-reception request
    payload.identifier = message.id;
    payload.data_length_code = TWAI_FRAME_MAX_DLC;
    
    std::copy(
        message.data.data(),
        message.data.data() + message.data.size(),
        payload.data
    );

    esp_err_t result = twai_transmit(&payload, pdMS_TO_TICKS(100));
    return result == ESP_OK;
}

Drivers::CanBus::Message Drivers::CanBus::receive(int retries) const {
    twai_message_t payload = {};
    for (int attempt = 0; true; ++attempt) {
        if (twai_receive(&payload, pdMS_TO_TICKS(1000)) == ESP_OK)
            break;
        if (attempt >= retries)
            return {};
    }

    Message message = {
        .id = payload.identifier,
        .data = {}
    };
    std::copy(
        payload.data,
        payload.data + payload.data_length_code,
        message.data.data()
    );
    return message;
}

} // namespace evms
