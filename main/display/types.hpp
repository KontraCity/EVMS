#pragma once

#include <cstdint>
#include <array>
#include <compare>
#include <algorithm>

namespace evms {

namespace Display {
    struct Size {
        int width = 0;
        int height = 0;

        constexpr size_t area() const {
            return static_cast<size_t>(width) * height;
        }

        constexpr explicit operator bool() const {
            return width && height;
        }

        constexpr std::strong_ordering operator<=>(const Size& other) const {
            size_t resolution = static_cast<size_t>(width) * height;
            size_t otherResolution = static_cast<size_t>(other.width) * other.height;
            return resolution <=> otherResolution;
        }
    };

    template <Size Dimensions>
    class PixelMap : public std::array<uint16_t, Dimensions.area()> {
    public:
        static constexpr int MapWidth = Dimensions.width;
        static constexpr int MapHeight = Dimensions.height;
        static constexpr Size MapSize = Dimensions;

    public:
        using std::array<uint16_t, Dimensions.area()>::array;

        constexpr PixelMap(std::initializer_list<uint16_t> init) {
            std::copy(init.begin(), init.end(), this->begin());
        }

    public:
        constexpr int width() const {
            return MapWidth;
        }

        constexpr int height() const {
            return MapHeight;
        }

        constexpr Size size() const {
            return MapSize;
        }

    public:
        constexpr explicit operator bool() const {
            return static_cast<bool>(MapSize);
        }
    };

    struct Position {
        int x = 0;
        int y = 0;

        constexpr explicit operator bool() const {
            return x >= 0 && y >= 0;
        }
    };
}

} // namespace evms
