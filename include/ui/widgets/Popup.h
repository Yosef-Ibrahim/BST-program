#pragma once
#include <string>
#include <raylib.h>
#include <ui/ThemeManager.h>
#include <utility/DrawHelpers.h>
#include <algorithm>

class Popup {
public:
    Popup();
    
    void show(const std::string& msg, Color color);
    void tick(float dt);
    void draw(int startX, int screenWidth, const Theme& theme);
    bool isActive() const;

private:
    std::string message;
    Color accentColor;
    float timer;
    float alpha;
    bool active;
};