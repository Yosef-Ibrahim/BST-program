#include <ui/widgets/IntInput.h>

IntInput::IntInput() : text(""), isActive(false) {}

bool IntInput::update(Rectangle bounds) {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        isActive = CheckCollisionPointRec(GetMousePosition(), bounds);
    }

    if (isActive) {
        int ch = GetCharPressed();
        while (ch > 0) {
            if ((ch >= '0' && ch <= '9') || (ch == '-' && text.empty())) {
                text += static_cast<char>(ch);
            }
            ch = GetCharPressed();
        }

        if (IsKeyPressed(KEY_BACKSPACE) && !text.empty()) {
            text.pop_back();
        }

        if (IsKeyPressed(KEY_ENTER) && !text.empty() && text != "-") {
            return true;
        }
    }

    return false;
}

void IntInput::draw(Rectangle bounds, const Theme& theme) {
    Color edgeCol = isActive ? theme.accent : theme.edgeCol;
    DrawHelpers::RRect(bounds, 0.2f, theme.bg, 10, edgeCol);

    std::string displayText = text;
    if (isActive && (int)(GetTime() * 2) % 2 == 0) displayText += "_";

    DrawText(displayText.c_str(), bounds.x + 10, bounds.y + 10, 20, theme.textMain);
}

int IntInput::getValue() const {
    return text.empty() || text == "-" ? 0 : std::stoi(text);
}

void IntInput::clear() {
    text = "";
}