#pragma once

#include <cstdint>
#include <string>

namespace evms {

namespace Drivers {
    class PwmLed {
    private:
        std::string m_logTag;
        int m_pin = 0;
        int m_channel = 0;

    public:
        PwmLed(int pin, int channel = 0);

        ~PwmLed();

    public:
        void setBrightness(uint8_t value);
    };
}

} // namespace evms
