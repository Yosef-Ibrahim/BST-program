#pragma once
#include <string>
#include <raylib.h>
#include <ThemeManager.h>

class IntInput {
public:
    IntInput(const std::string& defaultVal = "0");

    int getValue() const;
    
    // Call every frame; returns true if Enter was pressed
    bool update();
    
    // Draw the input box; returns true when clicked to acquire focus
    bool draw(int x, int y, int w, int h, const Theme& theme);

    void setFocus(bool isFocused);
    bool hasFocus() const;

private:
    std::string buf;
    bool focus;
};