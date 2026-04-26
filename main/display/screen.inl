namespace evms {

namespace Display {
    template <typename Map>
    void Screen::render(int x, int y, const Map& map) {
        Size mapSize = map.size();
        if (!mapSize) {
            // Map is empty. Can't draw!
            return;
        }

        if (x + mapSize.width <= 0 || x >= ScreenWidth) {
            // Map is out of display bounds x-wise!
            return;
        }
        if (y + mapSize.height <= 0 || y >= ScreenHeight) {
            // Map is out of display bounds y-wise!
            return;
        }

        int mapStride = mapSize.width;
        int widthStart = 0, heightStart = 0;
        if (x < 0) {
            widthStart = -x;
            mapSize.width -= widthStart;
            x = 0;
        }
        if (y < 0) {
            heightStart = -y;
            mapSize.height -= heightStart;
            y = 0;
        }

        if (x + (mapSize.width - widthStart) > ScreenWidth)
            mapSize.width = ScreenWidth + widthStart - x;
        if (y + (mapSize.height - heightStart) > ScreenHeight)
            mapSize.height = ScreenHeight + heightStart - y;

        // Column address set (X)
        int columnStart = x;
        int columnEnd = x + mapSize.width - 1;
        command(0x2A, {
            static_cast<uint8_t>(columnStart >> 8),
            static_cast<uint8_t>(columnStart & 0xFF),
            static_cast<uint8_t>(columnEnd >> 8),
            static_cast<uint8_t>(columnEnd & 0xFF)
        });

        // Row address set (Y)
        int rowStart = y;
        int rowEnd = y + mapSize.height - 1;
        command(0x2B, {
            static_cast<uint8_t>(rowStart >> 8),
            static_cast<uint8_t>(rowStart & 0xFF),
            static_cast<uint8_t>(rowEnd >> 8),
            static_cast<uint8_t>(rowEnd & 0xFF)
        });

        command(0x2C);
        m_dcPin.write(true);
        for (int row = 0; row < mapSize.height; ++row) {
            const uint16_t* mapRow = map.data() + ((heightStart + row) * mapStride) + widthStart;
            send(reinterpret_cast<const uint8_t*>(mapRow), mapSize.width * sizeof(uint16_t));
        }
    }
}

} // namespace evms
