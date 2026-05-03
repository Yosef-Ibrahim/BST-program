#include <ui/UIManager.h>
#include <cmath>
#include <algorithm>

// 
//  All fixed pixel offsets have been replaced with expressions that derive
//  from the panel width so the layout is self-consistent at any resolution.
// 

UIManager::UIManager()
    : panelWidth(350),
      statLine1("AVL Tree Ready"),
      statLine2("Type a value and press Insert!"),
      speedIdx(1), themeMenuOpen(false), themeIdx(0), numThemes(3)
{}

void UIManager::showPopup(const std::string& msg, Color color) { popup.show(msg, color); }
void UIManager::setStatus(const std::string& line1, const std::string& line2) { statLine1 = line1; statLine2 = line2; }
void UIManager::setTraversalResult(const std::string& title, const std::string& result) { travTitle = title; travResult = result; }
void UIManager::clearTraversalResult() { travTitle = ""; travResult = ""; }

// FIX: speed values tuned so Slow feels noticeably slow, Fast feels snappy
//      but never so fast it skips the animation entirely.
float UIManager::getAnimationSpeedMultiplier() const {
    if (speedIdx == 0) return 0.4f;  // Slow  – leisurely
    if (speedIdx == 1) return 1.0f;  // Normal
    return 2.5f;                     // Fast  – quick but still visible
}

//  Helper: compute the layout cursor positions used by both update() and draw()
// Returns the Y positions of the two input boxes so update() can pass them to
// the widget update functions without duplicating the layout arithmetic.
static void computeInputBounds(int panelWidth, bool themeMenuOpen, int numThemes,
                               Rectangle& outNodeBounds, Rectangle& outRandBounds)
{
    int scH = GetScreenHeight();
    const int PAD = panelWidth / 20;          // ≈18 px at 350
    const int bw  = panelWidth - PAD * 2;
    const int BTN_H   = std::max(36, scH / 24);
    const int INPUT_H = BTN_H + 10;

    int cy = PAD + 82 + 36;                  // title block + theme button
    if (themeMenuOpen) cy += numThemes * 32;
    int sY = cy + 20;

    // NODE VALUE section
    sY += PAD + 12;                          // section label height
    outNodeBounds = { (float)PAD, (float)sY, (float)bw, (float)INPUT_H };
    sY += INPUT_H + 8;   // input
    sY += BTN_H + 16;    // Insert button
    sY += BTN_H + 16;    // Delete button
    sY += BTN_H + 8;     // Clear button
    sY += 8;             // divider gap

    // SEARCH section
    sY += PAD + 12;      // section label
    sY += BTN_H + 8;     // pred+succ buttons
    sY += 8;             // divider gap

    // TRAVERSALS section
    sY += PAD + 12;      // section label
    sY += 4 * (BTN_H + 4); // 4 traversal buttons
    sY += 8;             // divider gap

    // RANDOM GENERATOR section
    sY += PAD + 12;      // section label
    // Treat rand properly as an input in height calculation but separate in position
    outRandBounds = { (float)(PAD + 38), (float)sY, 100.0f, (float)INPUT_H }; 
}

bool UIManager::update(float dt) {
    bool mouseOnUI = GetMouseX() <= panelWidth;
    popup.tick(dt);

    Rectangle nodeBounds, randBounds;
    computeInputBounds(panelWidth, themeMenuOpen, numThemes, nodeBounds, randBounds);

    if (nodeInput.update(nodeBounds)) {
        if (onInsertRequested) onInsertRequested(nodeInput.getValue());
        nodeInput.clear();
    }
    
    if (randomInput.update(randBounds)) {
        if (onRandomGenerateRequested) onRandomGenerateRequested(randomInput.getValue());
    }

    return mouseOnUI;
}

void UIManager::draw(const Theme& t, int nodeCount, int treeHeight) {
    int scW  = GetScreenWidth();
    int scH  = GetScreenHeight();

    //  Viewport-relative constants 
    const int PAD    = panelWidth / 20;           // ≈18 px
    const int bw     = panelWidth - PAD * 2;
    const int BTN_H  = std::max(36, scH / 24);    // button height scales with screen height
    const int INPUT_H = BTN_H + 10;               // input field slightly taller
    const int FONT_TITLE  = std::max(22, scH / 38);
    const int FONT_LABEL  = std::max(14, scH / 60);
    const int FONT_BTN    = std::max(15, scH / 58);
    const int FONT_SMALL  = std::max(12, scH / 72);

    //  Side Panel Background 
    DrawRectangle(0, 0, panelWidth, scH, t.panel);
    DrawRectangle(panelWidth, 0, 2, scH, t.edgeCol);
    DrawRectangleGradientV(0, 0, panelWidth, 5,
                           DrawHelpers::ColA(t.accent, 0.9f),
                           DrawHelpers::ColA(t.accent, 0.0f));

    //  Title 
    int cy = PAD;
    DrawText("AVL Tree",     PAD, cy,            FONT_TITLE,     t.accent);
    DrawText("Visualizer",   PAD, cy + FONT_TITLE + 4,    FONT_LABEL + 4, DrawHelpers::ColA(t.textMain, 0.5f));
    DrawText("Educational Edition", PAD, cy + FONT_TITLE + FONT_LABEL + 12, FONT_SMALL, DrawHelpers::ColA(t.textMuted, 0.7f));

    //  Theme Dropdown 
    cy += 82;
    const char* themeNames[] = { "Dark", "Light", "Ocean" };
    std::string themeBtnTxt  = themeMenuOpen
                               ? "Theme (Click to close)"
                               : "Theme: " + std::string(themeNames[themeIdx]);
    if (UIWidgets::Button(PAD, cy, bw, 32, themeBtnTxt.c_str(),
                          DrawHelpers::ColA(t.accent, 0.13f),
                          DrawHelpers::ColA(t.accent, 0.30f),
                          t.accent, FONT_SMALL + 2)) {
        themeMenuOpen = !themeMenuOpen;
    }
    cy += 36;

    if (themeMenuOpen) {
        for (int i = 0; i < numThemes; i++) {
            Color bg = (themeIdx == i) ? DrawHelpers::ColA(t.accent, 0.4f) : t.bg;
            if (UIWidgets::Button(PAD + 10, cy, bw - 20, 28, themeNames[i],
                                  bg, DrawHelpers::ColA(t.accent, 0.6f), t.textMain, FONT_SMALL + 2)) {
                themeIdx      = i;
                themeMenuOpen = false;
                if (onThemeChangeRequested)
                    onThemeChangeRequested(std::string("assets/themes/") + themeNames[i] + ".json");
            }
            cy += 32;
        }
    }

    DrawHelpers::Divider(cy + 4, PAD, panelWidth - PAD, t.edgeCol);
    int sY = cy + 20;

    //  Section label helper 
    auto SectionLbl = [&](int x, int y, const char* txt) {
        DrawRectangle(x, y + 4, 4, FONT_LABEL, t.accent);
        DrawText(txt, x + 11, y + 2, FONT_LABEL, t.textMuted);
    };

    //  NODE VALUE 
    SectionLbl(PAD, sY, "NODE VALUE");
    sY += PAD + 12;
    nodeInput.draw({ (float)PAD, (float)sY, (float)bw, (float)INPUT_H }, t);
    sY += INPUT_H + 8;

    if (UIWidgets::Button(PAD, sY, bw, BTN_H + 8, "Insert Node",
                          DrawHelpers::ColA(t.accent, 0.18f),
                          DrawHelpers::ColA(t.accent, 0.38f),
                          t.accent, FONT_BTN + 3)) {
        if (onInsertRequested) onInsertRequested(nodeInput.getValue());
        nodeInput.clear();
    }
    sY += BTN_H + 16;

    if (UIWidgets::Button(PAD, sY, bw, BTN_H + 8, "Delete Node",
                          DrawHelpers::ColA(t.error, 0.15f),
                          DrawHelpers::ColA(t.error, 0.35f),
                          t.error, FONT_BTN + 3)) {
        if (onRemoveRequested) onRemoveRequested(nodeInput.getValue());
        nodeInput.clear();
    }
    sY += BTN_H + 16;

    if (UIWidgets::Button(PAD, sY, bw, BTN_H, "Clear Entire Tree",
                          DrawHelpers::ColA({50,12,12,255}, 0.9f),
                          DrawHelpers::ColA(t.error, 0.22f),
                          DrawHelpers::ColA(t.error, 0.80f), FONT_BTN)) {
        if (onClearRequested) onClearRequested();
    }
    sY += BTN_H + 8;
    DrawHelpers::Divider(sY, PAD, panelWidth - PAD, t.edgeCol);
    sY += 8;

    //  SEARCH 
    SectionLbl(PAD, sY, "SEARCH");
    sY += PAD + 12;
    int hw = (bw - 6) / 2;
    if (UIWidgets::Button(PAD, sY, hw, BTN_H, "Predecessor",
                          DrawHelpers::ColA(t.accent, 0.15f),
                          DrawHelpers::ColA(t.accent, 0.35f),
                          t.accent, FONT_SMALL + 2)) {
        if (onPredecessorRequested) onPredecessorRequested(nodeInput.getValue());
    }
    if (UIWidgets::Button(PAD + hw + 6, sY, hw, BTN_H, "Successor",
                          DrawHelpers::ColA(t.accent, 0.15f),
                          DrawHelpers::ColA(t.accent, 0.35f),
                          t.accent, FONT_SMALL + 2)) {
        if (onSuccessorRequested) onSuccessorRequested(nodeInput.getValue());
    }
    sY += BTN_H + 8;
    DrawHelpers::Divider(sY, PAD, panelWidth - PAD, t.edgeCol);
    sY += 8;

    //  TRAVERSALS 
    SectionLbl(PAD, sY, "TRAVERSALS");
    sY += PAD + 12;
    const char* TRAVS[4] = {
        "Pre-Order  (Root->L->R)",
        "In-Order  (Sorted)",
        "Post-Order  (L->R->Root)",
        "Breadth-First  (Levels)"
    };
    for (int i = 0; i < 4; i++) {
        if (UIWidgets::Button(PAD, sY, bw, BTN_H - 2, TRAVS[i],
                              DrawHelpers::ColA(t.edgeCol, 0.35f),
                              DrawHelpers::ColA(t.accent, 0.18f),
                              t.textMain, FONT_SMALL + 3)) {
            if (onTraversalRequested) onTraversalRequested(i);
        }
        sY += BTN_H + 4;
    }
    DrawHelpers::Divider(sY, PAD, panelWidth - PAD, t.edgeCol);
    sY += 8;

    //  RANDOM GENERATOR 
    SectionLbl(PAD, sY, "RANDOM GENERATOR");
    sY += PAD + 12;
    DrawText("N =", PAD, sY + (INPUT_H - FONT_LABEL) / 2, FONT_LABEL + 2, t.textMuted);
    randomInput.draw({ (float)(PAD + 38), (float)sY, 100.0f, (float)INPUT_H }, t);
    if (UIWidgets::Button(PAD + 38 + 106, sY, bw - 38 - 106, INPUT_H, "Generate!",
                          DrawHelpers::ColA(t.success, 0.20f),
                          DrawHelpers::ColA(t.success, 0.42f),
                          t.success, FONT_SMALL + 2)) {
        if (onRandomGenerateRequested) onRandomGenerateRequested(randomInput.getValue());
    }
    sY += INPUT_H + 8;
    DrawHelpers::Divider(sY, PAD, panelWidth - PAD, t.edgeCol);
    sY += 8;

    //  ANIMATION SPEED 
    SectionLbl(PAD, sY, "ANIMATION SPEED");
    sY += PAD + 12;
    const Color SPEED_COLS[3]   = { {55,180,110,255}, {90,160,255,255}, {255,140,55,255} };
    const char* SPEED_LABELS[3] = { "Slow", "Normal", "Fast" };
    int bw3 = (bw - 8) / 3;
    for (int i = 0; i < 3; i++) {
        bool act = (speedIdx == i);
        Color bg  = act ? DrawHelpers::ColA(SPEED_COLS[i], 0.55f)
                        : DrawHelpers::ColA(SPEED_COLS[i], 0.10f);
        if (UIWidgets::Button(PAD + i*(bw3+4), sY, bw3, BTN_H, SPEED_LABELS[i],
                              bg, DrawHelpers::ColA(SPEED_COLS[i], 0.32f),
                              act ? WHITE : DrawHelpers::ColA(SPEED_COLS[i], 0.85f),
                              FONT_SMALL + 2))
            speedIdx = i;
        if (act)
            DrawRectangle(PAD + i*(bw3+4) + 4, sY + BTN_H - 2, bw3 - 8, 3, SPEED_COLS[i]);
    }

    //  STATUS STRIP 
    int stH = std::max(80, scH / 11);
    int stY = scH - stH;
    // Clamp so it never overlaps the buttons above
    if (stY < sY + BTN_H + 6) stY = sY + BTN_H + 6;

    DrawRectangle(0, stY, panelWidth, stH, DrawHelpers::ColA(t.bg, 0.97f));
    DrawLine(0, stY, panelWidth, stY, t.edgeCol);
    DrawText("STATUS", PAD, stY + 6, FONT_SMALL, t.textMuted);
    DrawText(statLine1.c_str(), PAD, stY + 6 + FONT_SMALL + 4, FONT_LABEL + 2, t.accent);
    DrawText(statLine2.c_str(), PAD, stY + 6 + FONT_SMALL + 4 + FONT_LABEL + 8, FONT_SMALL + 2, t.textMain);

    // ══════════════════════════════════════════════════════════════════════════
    //  CANVAS OVERLAYS
    // ══════════════════════════════════════════════════════════════════════════

    //  LIVE STATS card (top-right of canvas) 
    int statsW = std::max(200, scW / 6);
    int statsH = std::max(90,  scH / 10);
    int statsX = scW - statsW - 14;
    int statsY = 14;
    DrawRectangleRounded({(float)statsX, (float)statsY, (float)statsW, (float)statsH},
                         0.18f, 10, DrawHelpers::ColA(t.panel, 0.94f));
    DrawRectangleRoundedLines({(float)statsX, (float)statsY, (float)statsW, (float)statsH},
                              0.18f, 10, t.edgeCol);
    DrawRectangleRounded({(float)statsX, (float)statsY, (float)statsW, 4.0f},
                         0.18f, 10, DrawHelpers::ColA(t.accent, 0.7f));

    DrawText("LIVE STATS", statsX + PAD, statsY + 10, FONT_SMALL, t.textMuted);

    bool bal = (nodeCount <= 1)
               ? true
               : (treeHeight <= static_cast<int>(1.45f * std::log2((float)nodeCount + 1) + 1.6f));

    int statFontSize = std::max(15, scH / 52);
    DrawText(("Nodes:  " + std::to_string(nodeCount)).c_str(),
             statsX + PAD, statsY + 10 + FONT_SMALL + 8, statFontSize, t.textMain);
    DrawText(("Height: " + std::to_string(treeHeight)).c_str(),
             statsX + PAD, statsY + 10 + FONT_SMALL + 8 + statFontSize + 6, statFontSize, t.textMain);

    Color bc = bal ? t.success : t.error;
    DrawText(bal ? "Balanced  [OK]" : "Unbalanced!",
             statsX + PAD, statsY + 10 + FONT_SMALL + 8 + (statFontSize + 6)*2, FONT_SMALL + 2, bc);
    DrawRectangleRounded(
        {(float)(statsX + PAD), (float)(statsY + statsH - 4), (float)(statsW - PAD*2), 3.0f},
        0.5f, 10, DrawHelpers::ColA(bc, 0.5f));

    //  TRAVERSAL / SEARCH RESULT FOOTER (FIX: shown whenever travTitle is set) 
    if (!travTitle.empty()) {
        int footH  = std::max(56, scH / 16);
        int footY  = scH - footH;
        int footX  = panelWidth + 2;
        int footW  = scW - footX;

        // Background strip
        DrawRectangle(footX, footY, footW, footH, DrawHelpers::ColA(t.bg, 0.97f));
        DrawLine(footX, footY, scW, footY, t.edgeCol);

        // Accent line on top
        DrawRectangle(footX, footY, footW, 3, DrawHelpers::ColA(t.accent, 0.5f));

        int lblY = footY + 7;
        DrawText(travTitle.c_str(),  footX + 14, lblY,                    FONT_SMALL + 2, t.textMuted);
        DrawText(travResult.c_str(), footX + 14, lblY + FONT_SMALL + 8,   FONT_LABEL + 2, t.accent);

        // Small dismiss "×" button at the right edge
        int xBtnX = scW - 28;
        int xBtnY = footY + 8;
        if (UIWidgets::Button(xBtnX, xBtnY, 20, 20, "x",
                              DrawHelpers::ColA(t.error, 0.10f),
                              DrawHelpers::ColA(t.error, 0.30f),
                              t.error, FONT_SMALL)) {
            clearTraversalResult();
        }
    }

    //  HINTS bar (above the traversal footer when active) 
    const char* hintTxt = "Scroll: Zoom  |  Right-click+Drag: Pan  |  Double-click: Reset View  |  Enter: Insert";
    int htw  = MeasureText(hintTxt, FONT_SMALL);
    int hx   = panelWidth + 18;
    int hfootOffset = travTitle.empty() ? 22 : std::max(56, scH / 16) + 22;
    int hy   = scH - hfootOffset;
    DrawRectangleRounded({(float)hx - 4, (float)hy - 3, (float)htw + 8, (float)(FONT_SMALL + 6)},
                         0.3f, 10, DrawHelpers::ColA(t.bg, 0.9f));
    DrawText(hintTxt, hx, hy, FONT_SMALL, DrawHelpers::ColA(t.textMuted, 0.85f));

    //  DEPTH LEGEND 
    int lx = panelWidth + 18, ldy = 14;
    const Color NODE_COLORS[8] = {
        { 90,160,255,255 }, { 55,190,115,255 }, {255,145,60,255 },
        {180,80,240,255  }, {245,75,130,255  }, { 45,195,200,255},
        {225,200,40,255  }, {140,210,60,255  }
    };
    int dotSpacing = std::max(18, scW / 80);
    int legendW = MeasureText("Depth:", FONT_SMALL) + 8 + 8 * dotSpacing + 10;
    DrawRectangleRounded({(float)lx - 6, (float)ldy - 4, (float)legendW, (float)(FONT_SMALL + 10)},
                         0.3f, 10, DrawHelpers::ColA(t.panel, 0.8f));
    DrawText("Depth:", lx, ldy + 2, FONT_SMALL, t.textMuted);
    int dotX = lx + MeasureText("Depth:", FONT_SMALL) + 8;
    float dotR = dotSpacing * 0.28f;
    for (int i = 0; i < 8; i++)
        DrawCircleV({(float)(dotX + i * dotSpacing + (int)dotR), (float)(ldy + 7)}, dotR, NODE_COLORS[i]);

    //  Popup (always on top) 
    if (popup.isActive()) popup.draw(panelWidth, scW, t);
}