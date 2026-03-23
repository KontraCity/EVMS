#pragma once

#include "display/screen.hpp"

namespace evms {

namespace Graphics {
    void DrawLine(Display::Screen& screen, float x0, float y0, float x1, float y1, uint16_t color, float thickness);

    void DrawDirectedLine(Display::Screen& screen, float x, float y, float angle, float startDistance, float endDistance, uint16_t color, float thickness);
}

} // namespace evms
