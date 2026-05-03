#pragma once
#include <raylib.h>
#include <algorithm>

namespace DrawHelpers {
    // Returns a copy of the given color with a modified alpha
    Color ColA(Color c, float a);

    // Draws a rounded rectangle, optionally with an outline
    void RRect(Rectangle rect, float r, Color fill, int smooth, Color edge = {0,0,0,0});

    // Draws a simple horizontal divider line
    void Divider(int y, int x0, int x1, const Color& color);
}