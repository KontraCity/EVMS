#pragma once

#include <cstdint>
#include <array>
#include <string>

#include <driver/gpio.h>
#include <driver/twai.h>

namespace evms {

namespace Drivers {
    class CanBus {
    public:
        using Frame = std::array<uint8_t, TWAI_FRAME_MAX_DLC>;

        struct Message {
            uint32_t id = 0;
            Frame data = {};
        };

    private:
        std::string m_logTag;
        bool m_initialized = false;

    public:
        CanBus(const std::string& logName, gpio_num_t txPin, gpio_num_t rxPin, twai_mode_t mode = TWAI_MODE_NORMAL);

        CanBus(const CanBus& other) = delete;

        CanBus(CanBus&& other) noexcept;

        ~CanBus();

    public:
        CanBus& operator=(const CanBus& other) = delete;

        CanBus& operator=(CanBus&& other) noexcept;

    public:
        bool send(const Message& message) const;

        Message receive(int retries = 5) const;
    };
}

} // namespace evms
