#pragma once

#include <string>

#include <driver/gpio.h>

namespace evms {

namespace Drivers {
    class GpioPin {
    private:
        std::string m_logTag;
        gpio_num_t m_pin;
        gpio_mode_t m_mode;

    public:
        GpioPin(gpio_num_t pin, gpio_mode_t mode);

        ~GpioPin();

    public:
        void write(bool level);

        bool read() const;
    };
}

} // namespace evms
