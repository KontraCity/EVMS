#pragma once

#include <cstdint>
#include <array>
#include <compare>

namespace evms {

namespace Display {
    struct Dimensions2D {
        int width = 0;
        int height = 0;

        inline operator bool() const {
            return width && height;
        }

        inline std::strong_ordering operator<=>(const Dimensions2D& other) const {
            size_t resolution = width * height;
            size_t otherResolution = other.width * other.height;
            return resolution <=> otherResolution;
        }
    };

    template <Dimensions2D Dimensions>
    class PixelMap : public std::array<uint16_t, Dimensions.width * Dimensions.height> {
    public:
        using std::array<uint16_t, Dimensions.width * Dimensions.height>::array;

        constexpr PixelMap(std::initializer_list<uint16_t> init) {
            std::copy(init.begin(), init.end(), this->begin());
        }

    public:
        inline Dimensions2D dimensions() const {
            return Dimensions;
        }

    public:
        inline operator bool() const {
            return static_cast<bool>(Dimensions);
        }
    };
}

} // namespace evms
