#include "shapes.hpp"

#define _USE_MATH_DEFINES
#include <cmath>

#include "utility/math.hpp"

namespace evms {

void Graphics::DrawLine(Display::Screen& screen, float x0, float y0, float x1, float y1, uint16_t color, float thickness) {
    const float thicknessHalf = thickness * 0.5f;
    const float aaFalloff = 1.0f;
    const float xDistance = x1 - x0;
    const float yDistance = y1 - y0;
    const float length2 = xDistance * xDistance + yDistance * yDistance;
    if (length2 < 1e-6f) {
        // Don't draw lines that are way too small to avoid precision errors
        return;
    }

    const int minX = static_cast<int>(std::floor(std::min(x0, x1) - thicknessHalf - aaFalloff));
    const int minY = static_cast<int>(std::floor(std::min(y0, y1) - thicknessHalf - aaFalloff));
    const int maxX = static_cast<int>(std::ceil(std::max(x0, x1) + thicknessHalf + aaFalloff));
    const int maxY = static_cast<int>(std::ceil(std::max(y0, y1) + thicknessHalf + aaFalloff));

    for (int y = minY; y <= maxY; y++) {
        for (int x = minX; x <= maxX; x++) {
            float centerX = x + 0.5f;
            float centerY = y + 0.5f;

            float t = ((centerX - x0) * xDistance + (centerY - y0) * yDistance) / length2;
            t = Utility::Clamp(t, 0.0f, 1.0f);

            float projectedX = x0 + t * xDistance;
            float projectedY = y0 + t * yDistance;

            float xOffset = centerX - projectedX;
            float yOffset = centerY - projectedY;
            float distance = std::sqrt(xOffset * xOffset + yOffset * yOffset);

            float alpha = aaFalloff - (distance - thicknessHalf);
            if (alpha <= 0.0f)
                continue;

            float coverage = Utility::Clamp(alpha / aaFalloff, 0.0f, 1.0f);
            screen.blendPixelUnmarked(x, y, color, static_cast<uint8_t>(coverage * 255));
        }
    }

    screen.markChangedRegion(
        minX, minY,
        maxX - minX + 1,
        maxY - minY + 1
    );
}

void Graphics::DrawDirectedLine(Display::Screen& screen, float x, float y, float angle, float startDistance, float endDistance, uint16_t color, float thickness) {
    float rad = angle * M_PI / 180.0f;
    float cos = std::cos(rad);
    float sin = std::sin(rad);

    float x0 = x + cos * startDistance;
    float y0 = y + sin * startDistance;
    float x1 = x + cos * endDistance;
    float y1 = y + sin * endDistance;
    DrawLine(screen, x0, y0, x1, y1, color, thickness);
}

} // namespace evms
