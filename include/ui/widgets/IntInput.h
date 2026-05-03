#pragma once
#include <string>
#include <raylib.h>
#include <ui/ThemeManager.h>
#include <utility/DrawHelpers.h>

class IntInput {
public:
    IntInput();
    
    bool update(Rectangle bounds); 
    void draw(Rectangle bounds, const Theme& theme);
    
    int getValue() const;
    void clear();

private:
    std::string text;
    bool isActive;
};