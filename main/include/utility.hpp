#pragma once

// STL modules
#include <cstdio>
#include <cstring>
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
    /// @brief Delay execution for some time
    /// @param seconds Amount of seconds to delay for
    void Sleep(double seconds);

    /// @brief Request a string from serial port
    /// @param comment Request comment
    /// @return Received string
    std::string RequestString(const char* comment = "");

    /// @brief Request an integer from serial port
    /// @param comment Request comment
    /// @return Received integer
    int RequestInteger(const char* comment = "");

    /// @brief Request a float from serial port
    /// @param comment Request comment
    /// @return Received float
    float RequestFloat(const char* comment = "");

    /// @brief Generate random number
    /// @param min Min number value
    /// @param max Max number value
    /// @return Generated number
    int RandomNumber(int min, int max);

    /// @brief Constraint a value
    /// @param value The value to constraint
    /// @param min Minimum constraint limit
    /// @param max Maximum constraint limit
    /// @return Constrainted value
    float Constraint(float value, float min, float max);
}

} // namespace kc
