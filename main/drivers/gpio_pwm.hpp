#pragma once

#include <cstdint>
#include <string>

#include <driver/gpio.h>
#include <driver/ledc.h>

namespace evms {

namespace Drivers {
    class GpioPwm {
    private:
        static constexpr ledc_timer_bit_t DutyResolution = LEDC_TIMER_13_BIT;
        static constexpr int32_t MaxDuty = 0b1111111111111;

    private:
        std::string m_logTag;
        ledc_channel_t m_channel;
        gpio_num_t m_pin;

    public:
        GpioPwm(const char* logName, ledc_channel_t channel, gpio_num_t pin);

        GpioPwm(const GpioPwm& other) = delete;

        GpioPwm(GpioPwm&& other) noexcept;

        ~GpioPwm();

    public:
        GpioPwm& operator=(const GpioPwm& other) = delete;

        GpioPwm& operator=(GpioPwm&& other) noexcept;

    private:
        void setDutyRaw(int32_t duty);

    public:
        void setDuty(uint8_t value);

        void setDutyPercent(float percent);
    };
}

} // namespace evms
