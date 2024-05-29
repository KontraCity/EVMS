#pragma once

// STL modules
#include <cstdio>
#include <string>
#include <random>

// FreeRTOS modules
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// ESP modules
#include "esp_system.h"
#include "esp_console.h"
#include "esp_vfs_dev.h"
#include "esp_vfs_fat.h"
#include "esp_random.h"

// Drivers
#include "driver/uart.h"

// ROM modules
#include <rom/ets_sys.h>

namespace kc {

namespace Utility
{
    /// @brief Request a number from serial port
    /// @param comment Request comment
    /// @return Received number
    int RequestNumber(const char* comment);

    /// @brief Generate random number
    /// @param min Min number value
    /// @param max Max number value
    /// @return Generated number
    int RandomNumber(int min, int max);

    /// @brief Delay execution for some time
    /// @param seconds Amount of seconds to delay for
    void Sleep(double seconds);
}

} // namespace kc
