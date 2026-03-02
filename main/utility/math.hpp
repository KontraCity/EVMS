#pragma once

#include <type_traits>

namespace evms {

namespace Utility {
    template <typename T>
    inline T Constraint(T value, T min, T max) {
        if (value < min)
            value = min;
        else if (value > max)
            value = max;
        return value;
    }

    template <typename T>
    inline T ConvertRange(T value, T inMin, T inMax, T outMin, T outMax) {
        // Avoid division by zero
        if (inMax == inMin)
            return outMin;

        // Wider type for integers to allow maximum precision
        using Wide = std::conditional_t<std::is_integral_v<T>, int64_t, T>;
        Wide valueInRange = static_cast<Wide>(value - inMin);
        Wide outRange = static_cast<Wide>(outMax - outMin);
        Wide inRange = static_cast<Wide>(inMax - inMin);
        Wide result = valueInRange * outRange / inRange + outMin;
        return Constraint(static_cast<T>(result), outMin, outMax);
    }
}
    
} // namespace evms
