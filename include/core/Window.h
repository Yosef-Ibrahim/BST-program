#pragma once

#include <string>

#include <raylib.h>

class Window {
public:
    Window(int width, int height, const std::string& title, int targetFPS = 60);
    ~Window();

    bool shouldClose() const;
    void beginDrawing() const;
    void endDrawing() const;

    int getWidth() const;
    int getHeight() const;
};