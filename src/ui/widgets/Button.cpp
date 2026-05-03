#include <ui/widgets/Button.h>

namespace UIWidgets {

    bool Button(int x, int y, int w, int h, const char* label, 
                Color bg, Color hover, Color textCol, int fontSize) {
        
        Rectangle rect = { (float)x, (float)y, (float)w, (float)h };
        Vector2 mousePoint = GetMousePosition();
        
        bool isHovered = CheckCollisionPointRec(mousePoint, rect);
        bool isClicked = false;

        if (isHovered && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            isClicked = true;
        }

        Color currentBg = isHovered ? hover : bg;
        
        DrawHelpers::RRect(rect, 0.3f, currentBg, 10);

        int textW = MeasureText(label, fontSize);
        DrawText(label, x + (w - textW) / 2, y + (h - fontSize) / 2, fontSize, textCol);

        return isClicked;
    }
}