#include <graphics/TreeRenderer.h>

// ──────────────────────────────────────────────────────────────────────────────
//  All sizing is derived from the canvas dimensions so the tree looks correct
//  at any window resolution.  The canvas is the area to the right of the panel.
// ──────────────────────────────────────────────────────────────────────────────

void TreeRenderer::draw(Node<int>* root, const std::vector<int>& searchPath, const Theme& theme,
                        int screenWidth, int screenHeight, int panelWidth) {
    if (!root) return;

    // Canvas pixel dimensions
    float canvasW = static_cast<float>(screenWidth - panelWidth);
    float canvasH = static_cast<float>(screenHeight);

    // Horizontal span: the normalised x range [0,1] maps to this many pixels.
    // Using 2× canvas width gives the same over-scroll room as before.
    float spanX = canvasW * 1.5f;

    // Vertical spacing between depth levels.
    // Divide by 8 so up to 8 levels fit neatly on screen before the user pans.
    float spanY = canvasH / 8.0f;

    // Node radius scales with the smaller canvas dimension so it looks right
    // whether the window is wide or tall.
    float radius = std::min(canvasW, canvasH) * 0.026f;  // ≈25 px at 1280×1000
    radius = std::clamp(radius, 12.0f, 36.0f);

    drawNodeRecursive(root, 0, searchPath, theme, spanX, spanY, radius);
}

void TreeRenderer::drawNodeRecursive(Node<int>* node, int lvl, const std::vector<int>& path,
                                     const Theme& theme,
                                     float spanX, float spanY, float radius) {
    if (!node) return;

    // Normalised coordinates (0–1) → screen-space pixels
    Vector2 pos = { node->cur_x * spanX, node->cur_y * spanY };

    // ── Edge lines to children (drawn before the circle so they go behind) ──
    auto drawEdge = [&](Node<int>* child) {
        if (!child) return;
        Vector2 childPos = { child->cur_x * spanX, child->cur_y * spanY };
        float lineThick = std::max(1.5f, radius * 0.1f);
        DrawLineEx(pos, childPos, lineThick, DrawHelpers::ColA(theme.edgeCol, node->alpha));
        drawNodeRecursive(child, lvl + 1, path, theme, spanX, spanY, radius);
    };

    drawEdge(node->left);
    drawEdge(node->right);

    // ── Node circle ──
    // Rotation highlight: lerp accent → highlight colour while highlightVal > 0
    Color baseColor = theme.accent;
    if (node->isRotating && node->highlightVal > 0.0f) {
        Color hi = { 255, 220, 80, 255 }; // golden flash for rotations
        float t   = node->highlightVal;
        baseColor = {
            static_cast<unsigned char>(baseColor.r + (hi.r - baseColor.r) * t),
            static_cast<unsigned char>(baseColor.g + (hi.g - baseColor.g) * t),
            static_cast<unsigned char>(baseColor.b + (hi.b - baseColor.b) * t),
            255
        };
    }

    // New-node pulse: slightly enlarged radius while pulseTimer > 0
    float drawRadius = radius;
    if (node->isNew && node->pulseTimer > 0.0f) {
        drawRadius = radius * (1.0f + node->pulseTimer * 0.4f);
    }

    DrawCircleV(pos, drawRadius, DrawHelpers::ColA(baseColor, node->alpha));

    // Thin border ring – helps the node pop on both light and dark themes
    DrawCircleLines(static_cast<int>(pos.x), static_cast<int>(pos.y),
                    drawRadius,
                    DrawHelpers::ColA(theme.textMain, node->alpha * 0.25f));

    // ── Node label ──
    std::string text     = std::to_string(node->data);
    int         fontSize = static_cast<int>(radius * 0.85f);
    fontSize             = std::clamp(fontSize, 10, 28);

    int maxTextWidth = static_cast<int>(drawRadius * 2.0f) - 4;
    std::vector<std::string> lines = StringUtils::Wrap(text, maxTextWidth, fontSize);

    float totalH = static_cast<float>(lines.size() * fontSize);
    float startY = pos.y - totalH * 0.5f;

    for (size_t i = 0; i < lines.size(); ++i) {
        int   tw    = MeasureText(lines[i].c_str(), fontSize);
        float drawX = pos.x - tw * 0.5f;
        float drawY = startY + static_cast<float>(i * fontSize);
        DrawText(lines[i].c_str(),
                 static_cast<int>(drawX), static_cast<int>(drawY),
                 fontSize,
                 DrawHelpers::ColA(theme.textMain, node->alpha));
    }
}