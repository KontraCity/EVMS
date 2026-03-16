#pragma once

#include <cstdint>

#include "display/types.hpp"
#include "drivers/spi_bus.hpp"

namespace evms {

namespace Display {
    class Touch : private Drivers::SpiDevice {
    public:
        struct Calibration {
            bool calibrated   = false;
            int touchLeftLineX    = 0;
            int touchRightLineX   = 0;
            int touchTopLineY     = 0;
            int touchBottomLineY  = 0;
            int screenLeftLineX   = 0;
            int screenRightLineX  = 0;
            int screenTopLineY    = 0;
            int screenBottomLineY = 0;
        };

    private:
        Calibration m_calibration;

    public:
        Touch(const Drivers::SpiBus& spiBus, gpio_num_t csPin);

        Touch(const Touch& other) = delete;

        Touch(Touch&& other) noexcept;

        ~Touch() = default;

    public:
        Touch& operator=(const Touch& other) = delete;

        Touch& operator=(Touch&& other) noexcept;

    private:
        int getValue(uint8_t valueCode) const;

    public:
        void setCalibration(const Calibration& calibration);

        // { -1, -1 } if not touched
        Position getTouchPosition() const;

        // 0 if not touched
        int getTouchPressure() const;

        // In Celcius, ~5-10C error
        float getControllerTemp() const;
    };
}

} // namespace evms
