#pragma once

// STL modules
#include <vector>
#include <array>
#include <bitset>
#include <algorithm>
#include <cmath>
#include <cstring>

// ESP modules
#include <esp_log.h>

/* Forward types declaration for other modules */
namespace kc {
    namespace Display {
        // Image pixel map
        using BitMap = std::vector<std::vector<bool>>;

        // Static image pixel map
        template <int Width, int Height>
        using StaticBitMap = std::array<std::bitset<Width>, Height>;
    }
}

// Custom modules
#include "display/text.hpp"
#include "i2c.hpp"
#include "utility.hpp"

namespace kc {

namespace Display
{
    namespace MasterConst
    {
        constexpr const char* LogTag = "display";
        constexpr i2c_port_t I2CPort = I2C_NUM_0;
        constexpr uint8_t I2CAddress = 0x3C;

        namespace Dimensions
        {
            /*
            *   A typical 0.96" OLED display has a size of 128x64 pixels.
            *   It is divided in sectors, every sector is 1x8 pixels.
            */
            namespace Pixels
            {
                constexpr int Width = 128;
                constexpr int Height = 64;
            }

            namespace Sectors
            {
                constexpr int Width = Pixels::Width / 1;
                constexpr int Height = Pixels::Height / 8;
            }
        }

        namespace TransmissionFlags
        {
            constexpr uint8_t ShortCommand = 0x80;  // Single-byte command is transmitted
            constexpr uint8_t LongCommand = 0x00;   // A command of 2 or more bytes is transmitted
            constexpr uint8_t Data = 0x40;          // Data is transmitted
        }
    }

    class Master : public I2C::Device
    {
    private:
        // Image sector map
        using SectorMap = std::vector<std::vector<uint8_t>>;

    private:
        SectorMap m_displayMap;     // Data currently shown on the display
        SectorMap m_frameMap;       // Frame to be rendered to display

    public:
        /// @brief Initialize the display
        /// @param sclPin Pin I2C bus clock wire is connected to
        /// @param sdaPin Pin I2C bus data wire is connected to
        Master(int sclPin, int sdaPin);

        /// @brief Set contrast level
        /// @param level The contrast level to set
        void contrast(uint8_t level);

        /// @brief Turn display fill on or off
        /// @param enable Whether or not to turn the fill on
        void fill(bool enable);

        /// @brief Turn inverse effect on or off
        /// @param enable Whether or not to turn the effect on
        void inverse(bool enable);

        /// @brief Turn display on or off
        /// @param enable Whether or not to turn the display on
        void on(bool enable);

        /// @brief Clear the display
        /// @param x X coordinate of clear block's top left corner
        /// @param y Y coordinate of clear block's top left corner
        /// @param width Clear block width
        /// @param height Clear block height
        void clear(int x = 0, int y = 0, int width = MasterConst::Dimensions::Pixels::Width, int height = MasterConst::Dimensions::Pixels::Height);

        /// @brief Draw pixel map on frame
        /// @param x X coordinate of map's top left corner
        /// @param y Y coordinate of map's top left corner
        /// @param map The map to draw
        void draw(int x, int y, const BitMap& map);

        /// @brief Draw static map on frame
        /// @tparam Static map type
        /// @param x X coordinate of map's top left corner
        /// @param y Y coordinate of map's top left corner
        /// @param map The map to draw
        template <typename Type>
        void draw(int x, int y, const Type& map)
        {
            BitMap bitMap(map.size(), std::vector<bool>(map[0].size()));
            for (int y = 0, height = map.size(); y < height; ++y)
                for (int x = 0, width = map[0].size(); x < width; ++x)
                    bitMap[y][x] = map[y][width - x];
            draw(x, y, bitMap);
        }

        /// @brief Print text on frame
        /// @tparam Text font type
        /// @param x X coordinate of map's top left or top right corner
        /// @param y Y coordinate of map's top left or top right corner
        /// @param text The text to print
        /// @param topRightCorner Whether or not coordinates are of top right corner instead of top left
        template <typename Type>
        void print(int x, int y, const char* text, bool topRightCorner = false)
        {
            int length = std::strlen(text);
            if (length == 0)
                return;

            if (!topRightCorner)
            {
                for (int index = 0; index < length; ++index)
                {
                    Type map(text[index]);
                    draw(x + index * map[0].size(), y, map);
                }
            }
            else
            {
                for (int index = length - 1; index >= 0; --index)
                {
                    Type map(text[index]);
                    draw(x - (length - index) * map[0].size(), y, map);
                }
            }
        }

        /// @brief Render crafted frame on the display
        void render();
    };

}

} // namespace kc
