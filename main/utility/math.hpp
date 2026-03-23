#pragma once

#include <type_traits>

namespace evms {

namespace Utility {
    template <typename T>
    inline T Clamp(T value, T min, T max) {
        if (value < min)
            value = min;
        else if (value > max)
            value = max;
        return value;
    }

    template <typename T>
    inline T Map(T value, T inMin, T inMax, T outMin, T outMax, bool clamp = false) {
        // Avoid division by zero
        if (inMax == inMin)
            return outMin;

        // Wider type for integers to allow maximum precision
        using Wide = std::conditional_t<std::is_integral_v<T>, int64_t, T>;
        Wide valueInRange = static_cast<Wide>(value - inMin);
        Wide outRange = static_cast<Wide>(outMax - outMin);
        Wide inRange = static_cast<Wide>(inMax - inMin);
        T result = static_cast<T>(valueInRange * outRange / inRange + outMin);
        return clamp ? Clamp(result, outMin, outMax) : result;
    }
}
    
} // namespace evms
