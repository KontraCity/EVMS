#include "gpio_pin.hpp"

#include <utility>

#include <esp_log.h>

namespace evms {

static std::string MakeLogTag(const std::string& logName, gpio_num_t pin) {
    std::string pinStr = std::to_string(static_cast<int>(pin));
    return logName + " GpioPin [" + pinStr + "]";
}

static const char* ModeToString(gpio_mode_t mode) {
    switch (mode) {
        case GPIO_MODE_DISABLE:         return "disabled";
        case GPIO_MODE_INPUT:           return "input";
        case GPIO_MODE_OUTPUT:          return "output";
        case GPIO_MODE_OUTPUT_OD:       return "OD output";
        case GPIO_MODE_INPUT_OUTPUT_OD: return "OD bidirectional";
        case GPIO_MODE_INPUT_OUTPUT:    return "bidirectional";
        default:                        return "<unknown>";
    }
}

Drivers::GpioPin::GpioPin(const char* logName, gpio_num_t pin, gpio_mode_t mode)
    : m_logTag(MakeLogTag(logName, pin))
    , m_pin(pin) {
    gpio_config_t gpioConfig = {};
    gpioConfig.pin_bit_mask = 1ULL << m_pin;
    gpioConfig.mode = mode;
    gpioConfig.pull_up_en = GPIO_PULLUP_DISABLE;
    gpioConfig.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpioConfig.intr_type = GPIO_INTR_DISABLE;
    ESP_ERROR_CHECK(gpio_config(&gpioConfig));
    ESP_LOGI(m_logTag.c_str(), "Initialized with \"%s\" mode", ModeToString(mode));
}

Drivers::GpioPin::GpioPin(GpioPin&& other) noexcept
    : m_logTag(std::move(other.m_logTag)) 
    , m_pin(std::exchange(other.m_pin, GPIO_NUM_MAX))
{}

Drivers::GpioPin::~GpioPin() {
    if (m_pin != GPIO_NUM_MAX) {
        gpio_reset_pin(m_pin);
        ESP_LOGI(m_logTag.c_str(), "Deinitialized");
    }
}

Drivers::GpioPin& Drivers::GpioPin::operator=(GpioPin&& other) noexcept {
    if (&other != this) {
        m_logTag = std::move(other.m_logTag);
        m_pin = std::exchange(other.m_pin, GPIO_NUM_MAX);        
    }
    return *this;
}

void Drivers::GpioPin::write(bool level) {
    gpio_set_level(m_pin, level ? 1 : 0);
}

bool Drivers::GpioPin::read() const {
    return gpio_get_level(m_pin);
}

} // namespace evms
