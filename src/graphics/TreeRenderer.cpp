#include <graphics/TreeRenderer.h>

void TreeRenderer::draw(Node<int>* root, const std::vector<int>& searchPath, const Theme& theme, 
                        int screenWidth, int screenHeight, int panelWidth) {
    if (!root) return;

    float usableWidth = static_cast<float>(screenWidth - panelWidth);
    
    float canvasWidth = usableWidth * 2.0f; 

    float verticalSpacing = static_cast<float>(screenHeight) / 8.0f; 

    drawNodeRecursive(root, 0, searchPath, theme, canvasWidth, verticalSpacing);
}

void TreeRenderer::drawNodeRecursive(Node<int>* node, int lvl, const std::vector<int>& path, const Theme& theme, 
                                     float canvasWidth, float verticalSpacing) {
    if (!node) return;

    // fixed for now
    float radius = 25.0f;

    // Convert normalized coordinates (0.0 to 1.0) to screen coordinates
    Vector2 pos = { node->cur_x * canvasWidth, node->cur_y * verticalSpacing };

    // Draw connecting lines to children
    auto drawConnectedLine = [&](Node<int>* targetNode) {
        if (targetNode) {
            Vector2 childPos = { targetNode->cur_x * canvasWidth, targetNode->cur_y * verticalSpacing };
            DrawLineEx(pos, childPos, 3.0f, DrawHelpers::ColA(theme.edgeCol, node->alpha));
            drawNodeRecursive(targetNode, lvl + 1, path, theme, canvasWidth, verticalSpacing);
        }
    };

    drawConnectedLine(node->left);
    drawConnectedLine(node->right);

    // Draw the node circle
    Color nodeColor = theme.accent;
    Color finalColor = DrawHelpers::ColA(nodeColor, node->alpha);

    DrawCircleV(pos, radius, finalColor);

    // Draw the node value
    std::string text = std::to_string(node->data);
    int fontSize = 20;

    // The max width is the diameter of the circle, minus a tiny bit of padding
    int maxTextWidth = static_cast<int>(radius * 2.0f) - 4;

    // Get the wrapped lines
    std::vector<std::string> lines = StringUtils::Wrap(text, maxTextWidth, fontSize);

    // Calculate the total height of the text block so we can center it vertically
    float totalTextHeight = lines.size() * fontSize;
    float startY = pos.y - (totalTextHeight / 2.0f);

    // Draw each line centered horizontally
    for (size_t i = 0; i < lines.size(); ++i) {
        int textW = MeasureText(lines[i].c_str(), fontSize);
        
        float drawX = pos.x - (textW / 2.0f);
        float drawY = startY + (i * fontSize);

        DrawText(lines[i].c_str(), drawX, drawY, fontSize, DrawHelpers::ColA(theme.textMain, node->alpha));
    }
}