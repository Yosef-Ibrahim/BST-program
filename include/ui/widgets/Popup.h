#pragma once
#include <string>
#include <raylib.h>
#include <ThemeManager.h>

class Popup {
public:
    Popup();

    void show(const std::string& message, Color color, float duration = 3.5f);
    void tick(float dt);
    void draw(int panelWidth, int screenWidth, const Theme& theme) const;
    
    bool isActive() const;

private:
    std::string msg;
    float timer;
    float maxT;
    Color col;
};