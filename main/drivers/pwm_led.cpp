#include "pwm_led.hpp"

#include <driver/ledc.h>

#include <esp_log.h>

namespace evms {

Drivers::PwmLed::PwmLed(int pin, int channel)
    : m_logTag("PwmLed " + std::to_string(pin) + "/" + std::to_string(channel))
    , m_pin(pin)
    , m_channel(channel) {
    ledc_timer_config_t timerConfig = {};
    timerConfig.speed_mode = LEDC_LOW_SPEED_MODE;
    timerConfig.timer_num = LEDC_TIMER_0;
    timerConfig.duty_resolution = LEDC_TIMER_13_BIT;
    timerConfig.freq_hz = 1000;
    timerConfig.clk_cfg = LEDC_AUTO_CLK;
    ESP_ERROR_CHECK(ledc_timer_config(&timerConfig));

    ledc_channel_config_t channelConfig = {};
    channelConfig.speed_mode = LEDC_LOW_SPEED_MODE;
    channelConfig.channel = static_cast<ledc_channel_t>(m_channel);
    channelConfig.timer_sel = LEDC_TIMER_0;
    channelConfig.intr_type = LEDC_INTR_DISABLE;
    channelConfig.gpio_num = static_cast<gpio_num_t>(m_pin);
    channelConfig.duty = 0;
    channelConfig.hpoint = 0;
    ESP_ERROR_CHECK(ledc_channel_config(&channelConfig));
    
    ESP_LOGI(m_logTag.c_str(), "Initialized");
}

Drivers::PwmLed::~PwmLed() {
    ledc_stop(LEDC_LOW_SPEED_MODE, static_cast<ledc_channel_t>(m_channel), 0);
    gpio_reset_pin(static_cast<gpio_num_t>(m_pin));
    ESP_LOGI(m_logTag.c_str(), "Deinitialized");
}

void Drivers::PwmLed::setBrightness(uint8_t value) {
    int32_t duty = static_cast<int32_t>(0b1111111111111 * (value / 255.0));
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, static_cast<ledc_channel_t>(m_channel), duty));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, static_cast<ledc_channel_t>(m_channel)));
}

} // namespace evms
