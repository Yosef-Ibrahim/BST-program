#include <utility/DrawHelpers.h>

namespace DrawHelpers {
    Color ColA(Color c, float a) {
        return {c.r, c.g, c.b, static_cast<unsigned char>(std::clamp(a, 0.0f, 1.0f) * 255.0f)};
    }

    void RRect(float x, float y, float w, float h, float r, Color fill, int smooth,  Color edge) {
        Rectangle rect = {x, y, w, h};

        DrawRectangleRounded(rect, r, smooth, fill);

        if (edge.a > 0) {
            DrawRectangleRoundedLines({rect.x, rect.y, rect.width, rect.height}, r, smooth, edge);
        }
    }

    void Divider(int y, int x0, int x1, const Color& color) {
        DrawLine(x0, y, x1, y, color);
    }
}