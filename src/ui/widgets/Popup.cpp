#include <ui/widgets/Popup.h>

Popup::Popup() : timer(0.0f), alpha(0.0f), active(false) {}

void Popup::show(const std::string& msg, Color color) {
    message = msg;
    accentColor = color;
    timer = 2.5f;
    alpha = 1.0f;
    active = true;
}

void Popup::tick(float dt) {
    if (!active) return;

    if (timer > 0.0f) {
        timer -= dt;
    } else {
        alpha = std::clamp(alpha - dt * 2.0f, 0.0f, 1.0f);
        if (alpha == 0.0f) active = false;
    }
}

void Popup::draw(int startX, int screenWidth, const Theme& theme) {
    if (!active) return;

    int width = 300;
    int height = 50;

    int canvasWidth = screenWidth - startX;
    int x = startX + (canvasWidth - width) / 2;
    int y = 30;

    Color bg = DrawHelpers::ColA(theme.panel, alpha);
    Color textCol = DrawHelpers::ColA(theme.textMain, alpha);
    Color edge = DrawHelpers::ColA(accentColor, alpha);

    Rectangle rect = { (float)x, (float)y, (float)width, (float)height };
    DrawHelpers::RRect(rect, 0.4f, bg, 10, edge);

    int textW = MeasureText(message.c_str(), 20);
    DrawText(message.c_str(), x + (width - textW) / 2, y + 15, 20, textCol);
}

bool Popup::isActive() const { return active; }