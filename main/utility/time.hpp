#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <rom/ets_sys.h>

#include "esp_timer.h"

namespace evms {

namespace Utility {
    inline void Sleep(double seconds) {
        if (seconds <= 0)
            return;

        /*
        *   One FreeRTOS tick is 10ms, so 1 second is 100 ticks.
        *   Delays shorter than 1 tick should just burn CPU cycles.
        */
        if (seconds < 0.01)
            ets_delay_us(seconds * 1'000'000);
        vTaskDelay(seconds * 100);
    }

    inline double TimeSeconds() {
        int64_t microseconds = esp_timer_get_time();
        return microseconds / 1'000'000.0f;
    }
}

} // namespace evms
