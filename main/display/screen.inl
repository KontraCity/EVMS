namespace evms {

namespace Display {
    template <typename Map>
    void Screen::draw(int x, int y, const Map& map) {
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

        int widthStart = 0, heightStart = 0;
        if (x < 0) {
            widthStart = -x;
            x = 0;
        }
        if (y < 0) {
            heightStart = -y;
            y = 0;
        }

        int mapStride = mapSize.width;
        if (x + (mapSize.width - widthStart) > ScreenWidth)
            mapSize.width = ScreenWidth + widthStart - x;
        if (y + (mapSize.height - heightStart) > ScreenHeight)
            mapSize.height = ScreenHeight + heightStart - y;

        int colsToCopy = std::min<std::size_t>(
            mapSize.width - static_cast<std::size_t>(widthStart),
            ScreenWidth - static_cast<std::size_t>(x)
        );
        int rowsToCopy = std::min<std::size_t>(
            mapSize.height - static_cast<std::size_t>(heightStart),
            ScreenHeight - static_cast<std::size_t>(y)
        );

        for (int row = 0; row < rowsToCopy; ++row) {
            const uint16_t* mapRow = map.data() + ((heightStart + row) * mapStride) + widthStart;
            uint16_t* regionRow = s_framebuffer.data() + ((y + row) * ScreenWidth) + x;
            std::memcpy(regionRow, mapRow, colsToCopy * sizeof(uint16_t));
        }
        markChangedRegion(x, y, colsToCopy, rowsToCopy);
    }
}

} // namespace evms
