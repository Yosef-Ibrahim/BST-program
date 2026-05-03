#pragma once
#include <raylib.h>

#include <string>
#include <fstream>
#include <iostream>
#include <vector>

struct Theme {
    Color bg;
    Color panel;
    Color accent;
    Color success;
    Color error;
    Color textMain;
    Color textMuted;
    Color edgeCol;
};

class ThemeManager {
public:
    ThemeManager();
    
    const Theme& getCurrentTheme() const;

    bool loadFromFile(const std::string& filepath);

    void saveToFile();

private:
    Theme currentTheme;
};