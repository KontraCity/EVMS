#pragma once

// STL modules
#include <cmath>
#include <cstdio>

// ESP modules
#include "esp_system.h"
#include "esp_console.h"
#include "esp_vfs_dev.h"
#include "esp_vfs_fat.h"

// Drivers
#include "driver/uart.h"

// Custom modules
#include "display/master.hpp"

namespace kc {

namespace Display
{
    class Ui : public Master
    {
    private:
        /// @brief Create a "sliding value" widget
        /// @param value The value to create widget for
        /// @return Created widget
        static BitMap SlidingValueWidget(float value);

    public:
        /// @brief Initialize display UI
        /// @param sclPin Pin I2C bus clock wire is connected to
        /// @param sdaPin Pin I2C bus data wire is connected to
        Ui(int sclPin, int sdaPin);

        /// @brief Start display UI
        void start();
    };
}

} // namespace kc
