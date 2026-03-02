#include "pwm_led.hpp"

#include <esp_log.h>

#include "utility/math.hpp"

namespace evms {

static std::string MakeLogTag(ledc_channel_t channel, gpio_num_t pin) {
    std::string channelStr = std::to_string(static_cast<int>(channel));
    std::string pinStr = std::to_string(static_cast<int>(pin));
    return "PwmLed [C_" + channelStr + ", P_" + pinStr + "]";
}

Drivers::PwmLed::PwmLed(ledc_channel_t channel, gpio_num_t pin)
    : m_logTag(MakeLogTag(channel, pin))
    , m_channel(channel)
    , m_pin(pin) {
    ledc_timer_config_t timerConfig = {};
    timerConfig.speed_mode = LEDC_LOW_SPEED_MODE;
    timerConfig.timer_num = LEDC_TIMER_0;
    timerConfig.duty_resolution = LEDC_TIMER_13_BIT;
    timerConfig.freq_hz = 1000;
    timerConfig.clk_cfg = LEDC_AUTO_CLK;
    ESP_ERROR_CHECK(ledc_timer_config(&timerConfig));

    ledc_channel_config_t channelConfig = {};
    channelConfig.speed_mode = LEDC_LOW_SPEED_MODE;
    channelConfig.channel = m_channel;
    channelConfig.timer_sel = LEDC_TIMER_0;
    channelConfig.intr_type = LEDC_INTR_DISABLE;
    channelConfig.gpio_num = m_pin;
    channelConfig.duty = 0;
    channelConfig.hpoint = 0;
    ESP_ERROR_CHECK(ledc_channel_config(&channelConfig));
    
    ESP_LOGI(m_logTag.c_str(), "Initialized on timer 0");
}

Drivers::PwmLed::~PwmLed() {
    ledc_stop(LEDC_LOW_SPEED_MODE, m_channel, 0);
    gpio_reset_pin(m_pin);
    ESP_LOGI(m_logTag.c_str(), "Deinitialized");
}

void Drivers::PwmLed::setDutyRaw(int32_t duty) {
    duty = Utility::Constraint<uint32_t>(duty, 0, MaxDuty);
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, m_channel, duty));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, m_channel));
}

void Drivers::PwmLed::setDuty(uint8_t value) {
    setDutyRaw((static_cast<int32_t>(value) * MaxDuty) / 255);
}

void Drivers::PwmLed::setDutyPercent(float percent) {
    percent = Utility::Constraint(percent, 0.0f, 100.0f);
    setDutyRaw(static_cast<int32_t>((percent * MaxDuty) / 100.0f));
}

} // namespace evms
