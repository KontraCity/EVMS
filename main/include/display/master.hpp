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
        /// @param x X coordinate of top left corner
        /// @param y Y coordinate of top left corner
        /// @param map The map to draw
        void draw(int x, int y, const BitMap& map);

        /// @brief Print text on frame
        /// @param x X coordinate of top left corner
        /// @param y Y coordinate of top left corner
        /// @param text The text to print
        void print(int x, int y, const char* text);

        /// @brief Render crafted frame on the display
        void render();
    };

}

} // namespace kc
