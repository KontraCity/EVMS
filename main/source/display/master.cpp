#include "display/master.hpp"
using namespace kc::Display::MasterConst;

namespace kc {

Display::Master::Master(int sclPin, int sdaPin)
    : Device(I2CPort, I2CAddress, sclPin, sdaPin)
    , m_displayMap(Dimensions::Sectors::Width, std::vector<uint8_t>(Dimensions::Sectors::Height, 1))
    , m_frameMap(Dimensions::Sectors::Width, std::vector<uint8_t>(Dimensions::Sectors::Height, 0))
{
    send({ TransmissionFlags::LongCommand, 0x8D, 0x14 });   // Enable charge pump (OLED power supply)
    send({ TransmissionFlags::LongCommand, 0x20, 0b01 });   // Set VERTICAL addressing mode
    send({ TransmissionFlags::ShortCommand, 0xA1 });        // Map segments from left to right
    send({ TransmissionFlags::ShortCommand, 0xC8 });        // Map segments from top to bottom

    render();           // Clear garbage in display's RAM
    contrast(0x7F);     // Reset display contrast
    fill(false);        // Disable display fill
    inverse(false);     // Disable display inverse
    on(true);           // Turn the display on

    ESP_LOGI(LogTag, "Display is initialized");
}

void Display::Master::contrast(uint8_t level)
{
    /*
    *   "Set Contrast Control" command:
    *   == 1'0'0'0'0'0'0'1 == <-- The command
    *   == X'X'X'X'X'X'X'X == <-- The value
    *   -> Sets display contrast to the value of the second byte (0-255)
    */
    send({ TransmissionFlags::LongCommand, 0x81, level });
}

void Display::Master::fill(bool enable)
{
    /*
    *   "Entire Display ON" command:
    *   == 1'0'1'0'0'1'0'X ==
    *   -> Display is normal if X is 0 (command is 0xA4)
    *   -> Display is fully filled if X is 1 (command is 0xA5)
    */
    send({ TransmissionFlags::ShortCommand, static_cast<uint8_t>(0xA4 | enable) });
}

void Display::Master::inverse(bool enable)
{
    /*
    *   "Set Normal/Inverse Display" command:
    *   == 1'0'1'0'0'1'1'X ==
    *   -> Display is normal if X is 0 (command is 0xA6)
    *   -> Display is inversed if X is 1 (command is 0xA7)
    */
    send({ TransmissionFlags::ShortCommand, static_cast<uint8_t>(0xA6 | enable) });
}

void Display::Master::on(bool enable)
{
    /*
    *   "Set Display ON/OFF" command:
    *   == 1'0'1'0'1'1'1'X ==
    *   -> Display turns off if X is 0 (command is 0xAE)
    *   -> Display turns on if X is 1 (command is 0xAF)
    */
    send({ TransmissionFlags::ShortCommand, static_cast<uint8_t>(0xAE | enable) });
}

void Display::Master::clear(int x, int y, int width, int height)
{
    BitMap map(height, std::vector<bool>(width, 0));
    draw(x, y, map);
}

void Display::Master::draw(int x, int y, const BitMap& map)
{
    if (map.empty())
    {
        ESP_LOGE(LogTag, "Draw: Map is empty [x=%i, y=%i]", x, y);
        return;
    }

    int width = map[0].size(), height = map.size();
    for (const auto& array : map)
    {
        if (array.size() != width)
        {
            ESP_LOGE(
                LogTag, "Draw: Map width is not constant [x=%i, y=%i, size=%ix%i]",
                x, y, width, height
            );
            return;
        }
    }

    if (width == 0)
    {
        ESP_LOGE(
            LogTag, "Draw: Map width is 0 [x=%i, y=%i, size=%ix%i]",
            x, y, width, height
        );
        return;
    }

    if (x + width <= 0 || x >= Dimensions::Pixels::Width || y + height <= 0 || y >= Dimensions::Pixels::Height)
    {
        /* Nothing to draw, map is out of bounds */
        return;
    }

    int widthStart = 0, heightStart = 0;
    if (x < 0)
    {
        widthStart = -x;
        x = 0;
    }
    if (y < 0)
    {
        heightStart = -y;
        y = 0;
    }

    if (x + (width - widthStart) > Dimensions::Pixels::Width)
        width = Dimensions::Pixels::Width + widthStart - x;
    if (y + (height - heightStart) > Dimensions::Pixels::Height)
        height = Dimensions::Pixels::Height + heightStart - y;

    for (int pixelY = heightStart; pixelY < height; ++pixelY)
    {
        for (int pixelX = widthStart; pixelX < width; ++pixelX)
        {
            int byte = (pixelY - heightStart + y % 8) / 8;
            int bit = (pixelY - heightStart + y % 8) % 8;
            uint8_t& frameByte = m_frameMap[x + pixelX - widthStart][y / 8 + byte];
            frameByte = (frameByte & ~(1 << bit)) | (map[pixelY][pixelX] << bit);
        }
    }
}

void Display::Master::print(int x, int y, const char* text)
{
    for (int index = 0, length = std::strlen(text); index < length; ++index)
    {
        Font::CharacterMap characterMap = Font::GetCharacterMap(text[index]);
        BitMap bitMap(characterMap.size(), std::vector<bool>(characterMap[0].size()));
        for (int y = 0, height = characterMap.size(); y < height; ++y)
            for (int x = 0, width = characterMap[0].size(); x < width; ++x)
                bitMap[y][x] = characterMap[y][width - x];
        draw(x + index * characterMap[0].size(), y, bitMap);
    }
}

void Display::Master::render()
{
    int topCrop = -1, bottomCrop = -1;
    int leftCrop = -1, rightCrop = -1;
    for (int sectorX = 0; sectorX < m_frameMap.size(); ++sectorX)
    {
        bool columnChanged = false;
        for (int sectorY = 0; sectorY < m_frameMap[0].size(); ++sectorY)
        {
            if (m_frameMap[sectorX][sectorY] != m_displayMap[sectorX][sectorY])
            {
                if (topCrop == -1 || sectorY < topCrop)
                    topCrop = sectorY;
                if (sectorY + 1 > bottomCrop)
                    bottomCrop = sectorY + 1;
                columnChanged = true;
            }
        }

        if (columnChanged)
        {
            if (leftCrop == -1)
                leftCrop = sectorX;
            rightCrop = sectorX + 1;
        }
    }

    if (rightCrop == -1)
    {
        /* The frame and display contain the same data, no need to render */
        return;
    }

    send({ TransmissionFlags::LongCommand, 0x21, static_cast<uint8_t>(leftCrop), static_cast<uint8_t>(rightCrop - 1) });
    send({ TransmissionFlags::LongCommand, 0x22, static_cast<uint8_t>(topCrop), static_cast<uint8_t>(bottomCrop - 1) });

    std::vector<uint8_t> buffer = { TransmissionFlags::Data };
    buffer.reserve(1 + (rightCrop - leftCrop) * (bottomCrop - topCrop));
    for (int widthIndex = leftCrop; widthIndex < rightCrop; ++widthIndex)
    {
        for (int heightIndex = topCrop; heightIndex < bottomCrop; ++heightIndex)
        {
            buffer.push_back(m_frameMap[widthIndex][heightIndex]);
            m_displayMap[widthIndex][heightIndex] = m_frameMap[widthIndex][heightIndex];
        }
    }
    send(buffer);
}

} // namespace kc
