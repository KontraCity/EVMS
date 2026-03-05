#pragma once

#include <random>

#include "esp_random.h"

namespace evms {

namespace Utility {
    inline int RandomInteger(int min, int max) {
        static std::mt19937 generator(esp_random());
        return std::uniform_int_distribution(min, max)(generator);
    }

    inline float RandomFloat(float min, float max) {
        static std::mt19937 generator(esp_random());
        return std::uniform_real_distribution(min, max)(generator);
    }
}

} // namespace evms
