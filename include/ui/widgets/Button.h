#pragma once
#include <raylib.h>

namespace UIWidgets {
    bool Button(int x, int y, int w, int h, const char* label, 
                Color bg, Color hover, Color textCol, int fontSize = 20);
}