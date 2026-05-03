#include <core/Window.h>

Window::Window(int width, int height, const std::string& title, int targetFPS) {
    InitWindow(width, height, title.c_str());
    SetTargetFPS(targetFPS);
}

Window::~Window() {
    CloseWindow();
}

bool Window::shouldClose() const {
    return WindowShouldClose();
}

void Window::beginDrawing() const {
    BeginDrawing();
}

void Window::endDrawing() const {
    EndDrawing();
}

int Window::getWidth() const { return GetScreenWidth(); }
int Window::getHeight() const { return GetScreenHeight(); }