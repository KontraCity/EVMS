namespace evms {

namespace Display {
    template <typename Map>
    void Screen::draw(int x, int y, const Map& map) {
        Dimensions2D mapDimensions = map.dimensions();
        if (!mapDimensions) {
            // Map is empty. Can't draw!
            return;
        }

        if (x + mapDimensions.width <= 0 || x >= Dimensions.width) {
            // Map is out of display bounds x-wise!
            return;
        }
        if (y + mapDimensions.height <= 0 || y >= Dimensions.height) {
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

        int mapStride = mapDimensions.width;
        if (x + (mapDimensions.width - widthStart) > Dimensions.width)
            mapDimensions.width = Dimensions.width + widthStart - x;
        if (y + (mapDimensions.height - heightStart) > Dimensions.height)
            mapDimensions.height = Dimensions.height + heightStart - y;

        int colsToCopy = std::min<std::size_t>(
            mapDimensions.width - static_cast<std::size_t>(widthStart),
            Dimensions.width - static_cast<std::size_t>(x)
        );
        int rowsToCopy = std::min<std::size_t>(
            mapDimensions.height - static_cast<std::size_t>(heightStart),
            Dimensions.height - static_cast<std::size_t>(y)
        );

        for (int row = 0; row < rowsToCopy; ++row) {
            const uint16_t* mapRow = map.data() + ((heightStart + row) * mapStride) + widthStart;
            uint16_t* regionRow = s_framebuffer.data() + ((y + row) * Dimensions.width) + x;
            std::memcpy(regionRow, mapRow, colsToCopy * sizeof(uint16_t));
        }
        markChangedRegion(x, y, colsToCopy, rowsToCopy);
    }
}

} // namespace evms
