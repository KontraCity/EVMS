#pragma once

#include <string>

#include <driver/gpio.h>

namespace evms {

namespace Drivers {
    class GpioPin {
    private:
        std::string m_logTag;
        gpio_num_t m_pin;

    public:
        GpioPin(const char* logName, gpio_num_t pin, gpio_mode_t mode);

        GpioPin(const GpioPin& other) = delete;

        GpioPin(GpioPin&& other) noexcept;

        ~GpioPin();

    public:
        GpioPin& operator=(const GpioPin& other) = delete;

        GpioPin& operator=(GpioPin&& other) noexcept;

    public:
        void write(bool level);

        bool read() const;
    };
}

} // namespace evms
