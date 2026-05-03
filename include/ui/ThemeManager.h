#pragma once
#include <raylib.h>

struct Theme {
    Color bg, panel, panelEdge;
    Color accent, accent2, success, danger;
    Color textMain, textDim;
    Color nodeShadow, edgeCol;
    bool isNight;
};

class ThemeManager {
public:
    ThemeManager();
    
    void toggleMode();
    const Theme& getCurrentTheme() const;
    bool isNightMode() const;

private:
    bool nightMode;
    Theme nightTheme;
    Theme lightTheme;
};