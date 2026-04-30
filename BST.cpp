// ═══════════════════════════════════════════════════════════════════════════════
//  AVL Tree Visualizer  —  Educational Edition  v2
//
//  FIX #1  — Larger, clearer fonts throughout the UI
//  FIX #2  — Night / Light mode toggle button (default: Night)
//  FIX #3  — Node overlap for large trees: Reingold-Tilford layout in BST.h
//  FIX #4  — Negative number input: keyboard handler intercepts '-' key
// ═══════════════════════════════════════════════════════════════════════════════

#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "BST.h"

#include <string>
#include <algorithm>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <set>
#include <sstream>

// ─────────────────────────────────────────────────────────────────────────────
//  Layout
// ─────────────────────────────────────────────────────────────────────────────
static const int PANEL_W = 320;
static const int PAD     = 18;
static const int BTN_H   = 46;
static const int BTN_HSM = 38;

// ─────────────────────────────────────────────────────────────────────────────
//  Theme  (Night and Light palettes)
// ─────────────────────────────────────────────────────────────────────────────
struct Theme {
    Color bg, panel, panelEdge;
    Color accent, accent2, success, danger;
    Color textMain, textDim;
    Color nodeShadow, edgeCol;
    bool  isNight = true;   // used by popup and other widgets to pick bg
};

static const Theme NIGHT = {
    {  13,  15,  23, 255 },
    {  20,  24,  38, 255 },
    {  42,  52,  78, 255 },
    {  90, 160, 255, 255 },
    { 255, 145,  60, 255 },
    {  72, 215, 130, 255 },
    { 255,  75,  75, 255 },
    { 228, 233, 255, 255 },
    { 105, 118, 152, 255 },
    {   0,   0,   0,  80 },
    {  72,  84, 120, 255 },
    true,
};

static const Theme LIGHT = {
    { 240, 243, 252, 255 },
    { 255, 255, 255, 255 },
    { 195, 205, 225, 255 },
    {  45, 110, 230, 255 },
    { 210, 100,  20, 255 },
    {  30, 160,  90, 255 },
    { 210,  45,  45, 255 },
    {  22,  28,  50, 255 },
    { 120, 130, 155, 255 },
    {   0,   0,   0,  40 },
    { 130, 145, 175, 255 },
    false,
};

// Node colours (same set works well on both backgrounds)
static const Color NODE_COLORS[8] = {
    {  90, 160, 255, 255 },
    {  55, 190, 115, 255 },
    { 255, 145,  60, 255 },
    { 180,  80, 240, 255 },
    { 245,  75, 130, 255 },
    {  45, 195, 200, 255 },
    { 225, 200,  40, 255 },
    { 140, 210,  60, 255 },
};

// ─────────────────────────────────────────────────────────────────────────────
//  Colour utilities
// ─────────────────────────────────────────────────────────────────────────────
static inline Color ColA(Color c, float a) {
    c.a = (unsigned char)std::max(0.f, std::min(255.f, a * 255.f));
    return c;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Draw helpers
// ─────────────────────────────────────────────────────────────────────────────
static void RRect(float x, float y, float w, float h,
                  float r, Color fill, Color edge = {0,0,0,0}) {
    Rectangle rec = {x, y, w, h};
    DrawRectangleRounded(rec, r, 10, fill);
    if (edge.a > 0)
        DrawRectangleRoundedLines(rec, r, 10, edge);
}

static bool Btn(int x, int y, int w, int h, const char* lbl,
                Color bg, Color hov, Color tc, int fs = 20) {
    Rectangle rec = {(float)x,(float)y,(float)w,(float)h};
    bool over = CheckCollisionPointRec(GetMousePosition(), rec);
    bool clk  = over && IsMouseButtonReleased(MOUSE_LEFT_BUTTON);
    RRect(x, y, w, h, 0.22f, over ? hov : bg);
    if (over) DrawRectangleRoundedLines(rec, 0.22f, 10, ColA(tc, 0.6f));
    int tw = MeasureText(lbl, fs);
    DrawText(lbl, x + (w - tw) / 2, y + (h - fs) / 2, fs, tc);
    return clk;
}

static void SectionLbl(int x, int y, const char* txt, const Theme& T, int fs = 16) {
    DrawRectangle(x, y + 6, 4, 16, T.accent);
    DrawText(txt, x + 11, y + 4, fs, T.textDim);
}

static void Divider(int y, int x0, int x1, const Theme& T) {
    DrawLine(x0, y, x1, y, ColA(T.panelEdge, 0.65f));
}

// ─────────────────────────────────────────────────────────────────────────────
//  Word-wrap helper
// ─────────────────────────────────────────────────────────────────────────────
static std::vector<std::string> Wrap(const std::string& txt, int maxPx, int fs) {
    std::vector<std::string> lines;
    std::istringstream ss(txt);
    std::string word, line;
    while (ss >> word) {
        std::string test = line.empty() ? word : line + " " + word;
        if (MeasureText(test.c_str(), fs) <= maxPx) line = test;
        else { if (!line.empty()) lines.push_back(line); line = word; }
    }
    if (!line.empty()) lines.push_back(line);
    return lines;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Popup
// ─────────────────────────────────────────────────────────────────────────────
struct Popup {
    std::string msg;
    float timer = 0.f, maxT = 0.f;
    Color col = {};

    void show(const std::string& m, Color c, float dur = 3.5f) {
        msg = m; col = c; timer = maxT = dur;
    }
    bool active() const { return timer > 0.f; }
    void tick(float dt) { timer = std::max(0.f, timer - dt); }

    void draw(int pW, int scW, const Theme& T) const {
        if (!active()) return;
        float fade = std::min(1.f, timer * 3.f);
        int bw = std::min(600, scW - pW - 40);
        int bh = 56, bx = pW + (scW - pW - bw) / 2, by = 18;
        // Theme-aware background
        Color popupBg = T.isNight
            ? ColA({12,16,28,245}, fade)
            : ColA({220,224,238,245}, fade);
        RRect(bx, by, bw, bh, 0.35f, popupBg);
        DrawRectangleRoundedLines({(float)bx,(float)by,(float)bw,(float)bh},
                                  0.35f, 10, ColA(col, fade * 0.85f));
        float prog = (maxT > 0.f) ? timer / maxT : 0.f;
        RRect(bx+10, by+bh-5, (int)((bw-20)*prog), 3, 0.5f, ColA(col, fade*0.55f));
        DrawText(msg.c_str(), bx + (bw - MeasureText(msg.c_str(),21))/2,
                 by + (bh-21)/2 - 2, 21, ColA(T.textMain, fade));
    }
};

// ─────────────────────────────────────────────────────────────────────────────
//  Draw the tree (uses cur_x/cur_y for smooth motion)
// ─────────────────────────────────────────────────────────────────────────────
static void DrawTree(Node<int>* n, int lvl,
                     const std::vector<int>& path,
                     const Theme& T)
{
    if (!n) return;
    float cx = n->cur_x, cy = n->cur_y, a = n->alpha;

    // Edges first
    if (n->left) {
        float ea = std::min(a, n->left->alpha);
        DrawLineEx({cx,cy},{n->left->cur_x, n->left->cur_y},
                   2.4f, ColA(T.edgeCol, ea * 0.82f));
        DrawTree(n->left, lvl+1, path, T);
    }
    if (n->right) {
        float ea = std::min(a, n->right->alpha);
        DrawLineEx({cx,cy},{n->right->cur_x, n->right->cur_y},
                   2.4f, ColA(T.edgeCol, ea * 0.82f));
        DrawTree(n->right, lvl+1, path, T);
    }

    Color base = ColA(NODE_COLORS[lvl % 8], a);

    // Pulse ring (new insert)
    if (n->isNew && n->pulseTimer > 0.f) {
        float p = std::sin(n->pulseTimer * 7.f) * 0.5f + 0.5f;
        DrawCircleV({cx,cy}, 30.f + p*12.f, ColA(base, 0.22f * a));
    }
    // Rotation glow
    if (n->isRotating && n->highlightVal > 0.f)
        DrawCircleV({cx,cy}, 36.f, ColA({255,200,50,255}, n->highlightVal*0.5f*a));

    // Path highlight
    bool inPath = std::find(path.begin(), path.end(), n->data) != path.end();
    if (inPath)
        DrawCircleV({cx,cy}, 33.f, ColA({255,230,40,255}, 0.42f*a));

    // Shadow
    DrawCircleV({cx+3.f,cy+4.f}, 24.f, ColA(T.nodeShadow, a*0.5f));
    // Main
    DrawCircleV({cx,cy}, 24.f, base);
    // Rim shimmer
    DrawCircleV({cx-5.f,cy-6.f}, 10.f, ColA(WHITE, 0.08f*a));
    DrawCircleLinesV({cx,cy}, 24.f, ColA(WHITE, 0.18f*a));

    // Value — FIX #1: larger font in nodes
    std::string val = std::to_string(n->data);
    int fs = (val.size() > 4) ? 13 : (val.size() > 3) ? 15 : (val.size() > 2) ? 17 : 20;
    DrawText(val.c_str(),
             (int)(cx - MeasureText(val.c_str(),fs)/2.f),
             (int)(cy - fs/2.f), fs, ColA(WHITE, a));

    // Height badge
    std::string hb = "h" + std::to_string(n->height);
    DrawText(hb.c_str(), (int)(cx+16.f), (int)(cy-26.f), 12, ColA(T.textDim, 0.7f*a));
}

// ─────────────────────────────────────────────────────────────────────────────
//  FIX #4 — Manual integer input that accepts minus sign
//  We manage our own string buffer so we're not at the mercy of GuiValueBox
// ─────────────────────────────────────────────────────────────────────────────
struct IntInput {
    std::string buf   = "0";
    bool        focus = false;

    int  value() const {
        if (buf.empty() || buf == "-") return 0;
        try { return std::stoi(buf); } catch (...) { return 0; }
    }

    // Call every frame; returns true if Enter was pressed
    bool update() {
        if (!focus) return false;
        // Digits
        for (int k = KEY_ZERO; k <= KEY_NINE; ++k) {
            if (IsKeyPressed(k)) {
                if (buf == "0") buf = "";
                buf += (char)('0' + (k - KEY_ZERO));
            }
        }
        // Numpad
        for (int k = KEY_KP_0; k <= KEY_KP_9; ++k) {
            if (IsKeyPressed(k)) {
                if (buf == "0") buf = "";
                buf += (char)('0' + (k - KEY_KP_0));
            }
        }
        // Minus sign — FIX #4
        if (IsKeyPressed(KEY_MINUS) || IsKeyPressed(KEY_KP_SUBTRACT)) {
            if (buf.empty() || buf == "0") buf = "-";
            else if (buf[0] == '-') buf = buf.substr(1);  // toggle
            else buf = "-" + buf;
        }
        // Backspace
        if (IsKeyPressed(KEY_BACKSPACE)) {
            if (!buf.empty()) buf.pop_back();
            if (buf.empty()) buf = "0";
        }
        // Clamp to -9999..9999
        if (buf != "-" && !buf.empty()) {
            int v = 0;
            try { v = std::stoi(buf); } catch (...) {}
            if (v >  9999) buf = "9999";
            if (v < -9999) buf = "-9999";
        }
        return IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER);
    }

    // Draw the input box; returns true when clicked (to acquire focus)
    bool draw(int x, int y, int w, int h, const Theme& T) {
        Rectangle rec = {(float)x,(float)y,(float)w,(float)h};
        bool hov = CheckCollisionPointRec(GetMousePosition(), rec);
        bool clk = hov && IsMouseButtonReleased(MOUSE_LEFT_BUTTON);

        Color border = focus ? ColA(T.accent, 0.95f) : T.panelEdge;
        Color bg     = focus ? ColA(T.accent, 0.10f) : ColA(WHITE, 0.04f);
        RRect(x, y, w, h, 0.18f, bg, border);

        // Blinking cursor
        std::string disp = buf;
        if (focus && ((int)(GetTime() * 2) % 2 == 0)) disp += "|";

        int fs = 22;   // FIX #1: bigger font in input
        int tw = MeasureText(disp.c_str(), fs);
        DrawText(disp.c_str(), x + (w - tw) / 2, y + (h - fs) / 2, fs, T.textMain);

        if (clk) focus = true;
        return clk;
    }
};

// ─────────────────────────────────────────────────────────────────────────────
//  main
// ─────────────────────────────────────────────────────────────────────────────
int main() {
    std::srand((unsigned)std::time(nullptr));

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(1280, 820, "AVL Tree Visualizer  |  Educational Edition");
    MaximizeWindow();
    SetTargetFPS(60);

    BST<int> tree;

    // ── UI state ──────────────────────────────────────────────────────────
    IntInput valInput;
    IntInput randInput;
    randInput.buf = "12";

    // FIX #2 — theme (night = default)
    bool nightMode = true;

    std::string travTitle = "", travResult = "";
    std::string statLine1 = "AVL Tree Ready";
    std::string statLine2 = "Type a value and press Insert!";

    Popup popup;

    // Animation speed
    static const float SPEED_TABLE[3]  = { 3.f, 9.f, 22.f };
    static const char* SPEED_LABELS[3] = { "Slow", "Normal", "Fast" };
    int speedIdx = 0;

    // Camera
    Camera2D cam = {}; cam.zoom = 1.f;
    bool camReady = false;

    // ─────────────────────────────────────────────────────────────────────
    //  Main loop
    // ─────────────────────────────────────────────────────────────────────
    while (!WindowShouldClose()) {
        float dt  = GetFrameTime();
        int   scW = GetScreenWidth();
        int   scH = GetScreenHeight();

        const Theme& T = nightMode ? NIGHT : LIGHT;

        if (!camReady) {
            cam.offset = { PANEL_W + (scW - PANEL_W) * 0.5f, scH * 0.35f };
            cam.target = { 0.f, 0.f };
            camReady   = true;
        }

        // ── Tick ──────────────────────────────────────────────────────────
        popup.tick(dt);
        tree.tickAnimations(dt, SPEED_TABLE[speedIdx]);

        // ── Zoom ──────────────────────────────────────────────────────────
        float wheel = GetMouseWheelMove();
        if (wheel != 0 && GetMousePosition().x > PANEL_W) {
            Vector2 mw = GetScreenToWorld2D(GetMousePosition(), cam);
            cam.offset = GetMousePosition();
            cam.target = mw;
            float f    = (wheel > 0) ? 1.12f : (1.f/1.12f);
            cam.zoom   = std::max(0.08f, std::min(6.f, cam.zoom * f));
        }

        // ── Pan ───────────────────────────────────────────────────────────
        if ((IsMouseButtonDown(MOUSE_BUTTON_RIGHT) || IsMouseButtonDown(MOUSE_BUTTON_MIDDLE))
            && GetMousePosition().x > PANEL_W)
        {
            Vector2 d = GetMouseDelta();
            cam.target.x -= d.x / cam.zoom;
            cam.target.y -= d.y / cam.zoom;
        }

        // ── Double-click to reset view ─────────────────────────────────
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && GetMousePosition().x > PANEL_W) {
            static double lastC = 0.0;
            double now = GetTime();
            if (now - lastC < 0.32) {
                cam.target = {0.f, 0.f}; cam.zoom = 1.f;
                cam.offset = { PANEL_W + (scW - PANEL_W) * 0.5f, scH * 0.35f };
            }
            lastC = now;
        }

        // ── Deactivate inputs on outside click ────────────────────────────
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mp = GetMousePosition();
            if (!CheckCollisionPointRec(mp, {(float)PAD, 122.f, 260.f, 46.f}))
                valInput.focus  = false;
            if (!CheckCollisionPointRec(mp, {84.f, 640.f, 110.f, 38.f}))
                randInput.focus = false;
        }

        // ── FIX #4: input updates ─────────────────────────────────────────
        bool enterVal  = valInput.update();
        randInput.update();

        // Enter = quick insert
        if (enterVal) {
            std::string rotMsg;
            tree.searchPath.clear();
            tree.insert(valInput.value(), rotMsg);
            std::string m = "Inserted: " + std::to_string(valInput.value());
            if (!rotMsg.empty()) m += "  (" + rotMsg + ")";
            popup.show(m, rotMsg.empty() ? T.success : T.accent2);
            statLine1 = "Inserted " + std::to_string(valInput.value());
            statLine2 = rotMsg.empty() ? "No rotation needed" : rotMsg;
            travTitle = travResult = "";
        }

        // ══════════════════════════════════════════════════════════════════
        //  RENDER
        // ══════════════════════════════════════════════════════════════════
        BeginDrawing();
        ClearBackground(T.bg);

        // ── Tree canvas ───────────────────────────────────────────────────
        BeginMode2D(cam);
        DrawTree(tree.getRoot(), 0, tree.searchPath, T);
        EndMode2D();

        // ── Side panel ────────────────────────────────────────────────────
        DrawRectangle(0, 0, PANEL_W, scH, T.panel);
        DrawRectangle(PANEL_W, 0, 2, scH, T.panelEdge);
        DrawRectangleGradientV(0, 0, PANEL_W, 5, ColA(T.accent, 0.9f), ColA(T.accent, 0.f));

        // ─── Title ───────────────────────────────────────────────────────
        int cy = 14;
        DrawText("AVL Tree",    PAD, cy,      34, T.accent);
        DrawText("Visualizer",  PAD, cy + 38, 22, ColA(T.textMain, 0.5f));
        DrawText("Educational Edition", PAD, cy + 63, 14, ColA(T.textDim, 0.7f));

        // ─── Night / Light toggle — full-width row below title ───────────
        {
            const char* modeLabel = nightMode ? "Switch to Light Mode" : "Switch to Night Mode";
            Color modeAccent = nightMode ? Color{255, 210, 60, 255} : Color{60, 100, 220, 255};
            Color modeBg     = ColA(modeAccent, 0.13f);
            Color modeHov    = ColA(modeAccent, 0.30f);
            if (Btn(PAD, cy + 82, PANEL_W - PAD * 2, 32, modeLabel,
                    modeBg, modeHov, modeAccent, 15))
                nightMode = !nightMode;
        }

        Divider(cy + 122, PAD, PANEL_W - PAD, T);

        // ─── NODE VALUE input ─────────────────────────────────────────────
        int sY = cy + 134;
        SectionLbl(PAD, sY, "NODE VALUE", T, 17);
        sY += 30;

        valInput.draw(PAD, sY, 260, 46, T);          // FIX #1+#4
        sY += 54;

        // Insert
        if (Btn(PAD, sY, 260, BTN_H, "Insert Node",
                ColA(T.accent, 0.18f), ColA(T.accent, 0.38f), T.accent, 21))   // FIX #1
        {
            std::string rotMsg;
            tree.searchPath.clear();
            tree.insert(valInput.value(), rotMsg);
            std::string m = "Inserted: " + std::to_string(valInput.value());
            if (!rotMsg.empty()) m += "  (" + rotMsg + ")";
            popup.show(m, rotMsg.empty() ? T.success : T.accent2);
            statLine1 = "Inserted " + std::to_string(valInput.value());
            statLine2 = rotMsg.empty() ? "No rotation needed" : rotMsg;
            travTitle = travResult = "";
        }
        sY += BTN_H + 6;

        // Delete
        if (Btn(PAD, sY, 260, BTN_H, "Delete Node",
                ColA(T.danger, 0.15f), ColA(T.danger, 0.35f), T.danger, 21))   // FIX #1
        {
            std::string rotMsg;
            tree.searchPath.clear();
            tree.remove(valInput.value(), rotMsg);
            std::string m = "Deleted: " + std::to_string(valInput.value());
            if (!rotMsg.empty()) m += "  (" + rotMsg + ")";
            popup.show(m, T.danger);
            statLine1 = "Deleted " + std::to_string(valInput.value());
            statLine2 = rotMsg.empty() ? "No rotation needed" : rotMsg;
            travTitle = travResult = "";
        }
        sY += BTN_H + 6;

        // Clear
        if (Btn(PAD, sY, 260, BTN_HSM, "Clear Entire Tree",
                ColA({50,12,12,255}, 0.9f), ColA(T.danger, 0.22f),
                ColA(T.danger, 0.80f), 18))                                      // FIX #1
        {
            tree.clearTree(); tree.searchPath.clear();
            travTitle = travResult = "";
            statLine1 = "Tree cleared"; statLine2 = "Start fresh!";
            popup.show("Tree cleared!", T.danger, 2.f);
        }
        sY += BTN_HSM + 8;
        Divider(sY, PAD, PANEL_W - PAD, T);

        // ─── SEARCH ───────────────────────────────────────────────────────
        sY += 8;
        SectionLbl(PAD, sY, "SEARCH", T, 17);
        sY += 30;

        int hw = (260 - 6) / 2;
        if (Btn(PAD, sY, hw, BTN_HSM, "Predecessor",
                ColA(T.accent2, 0.15f), ColA(T.accent2, 0.35f), T.accent2, 17))
        {
            int pv = tree.getPredecessor(valInput.value());
            std::string m = (pv != -1)
                ? "Pred(" + std::to_string(valInput.value()) + ") = " + std::to_string(pv)
                : "No predecessor for " + std::to_string(valInput.value());
            popup.show(m, T.accent2); statLine1 = m; statLine2 = "Path shown in yellow";
        }
        if (Btn(PAD + hw + 6, sY, hw, BTN_HSM, "Successor",
                ColA(T.accent2, 0.15f), ColA(T.accent2, 0.35f), T.accent2, 17))
        {
            int sv = tree.getSuccessor(valInput.value());
            std::string m = (sv != -1)
                ? "Succ(" + std::to_string(valInput.value()) + ") = " + std::to_string(sv)
                : "No successor for " + std::to_string(valInput.value());
            popup.show(m, T.accent2); statLine1 = m; statLine2 = "Path shown in yellow";
        }
        sY += BTN_HSM + 8;
        Divider(sY, PAD, PANEL_W - PAD, T);

        // ─── TRAVERSALS ───────────────────────────────────────────────────
        sY += 8;
        SectionLbl(PAD, sY, "TRAVERSALS", T, 17);
        sY += 30;

        struct TravItem { const char* lbl; };
        static const TravItem TRAVS[4] = {
            {"Pre-Order  (Root->L->R)"},
            {"In-Order  (Sorted)"},
            {"Post-Order  (L->R->Root)"},
            {"Breadth-First  (Levels)"},
        };
        for (int i = 0; i < 4; i++) {
            if (Btn(PAD, sY, 260, 36, TRAVS[i].lbl,
                    ColA(T.panelEdge, 0.35f), ColA(T.accent, 0.18f), T.textMain, 17))
            {
                switch(i) {
                    case 0: travTitle="Pre-Order:";    travResult=tree.getPreOrderString();    break;
                    case 1: travTitle="In-Order:";     travResult=tree.getInOrderString();     break;
                    case 2: travTitle="Post-Order:";   travResult=tree.getPostOrderString();   break;
                    case 3: travTitle="Breadth-First:";travResult=tree.getBreadthFirstString();break;
                }
            }
            sY += 40;
        }
        Divider(sY, PAD, PANEL_W - PAD, T);

        // ─── RANDOM GENERATOR ─────────────────────────────────────────────
        sY += 8;
        SectionLbl(PAD, sY, "RANDOM GENERATOR", T, 17);
        sY += 30;

        DrawText("N =", PAD, sY + 9, 18, T.textDim);   // FIX #1
        randInput.draw(PAD + 38, sY, 100, 38, T);

        if (Btn(PAD + 38 + 106, sY, 260 - 38 - 106, 38, "Generate!",
                ColA(T.success, 0.20f), ColA(T.success, 0.42f), T.success, 18))
        {
            int count = randInput.value();
            if (count < 1)  count = 1;
            if (count > 99) count = 99;

            tree.clearTree(); tree.searchPath.clear();
            std::set<int> used;
            int ins = 0, att = 0;
            while (ins < count && att < count * 30) {
                int v = (rand() % 199) - 99;
                if (!used.count(v)) {
                    used.insert(v);
                    std::string dummy;
                    tree.insert(v, dummy);
                    ins++;
                }
                att++;
            }
            popup.show("Generated " + std::to_string(ins) + " random nodes!", T.success);
            statLine1 = "Random tree: " + std::to_string(ins) + " nodes";
            statLine2 = "Add / delete nodes freely!";
            travTitle = travResult = "";
        }
        sY += 46;
        Divider(sY, PAD, PANEL_W - PAD, T);

        // ─── ANIMATION SPEED ──────────────────────────────────────────────
        sY += 8;
        SectionLbl(PAD, sY, "ANIMATION SPEED", T, 17);
        sY += 30;

        static const Color SPEED_COLS[3] = {
            { 55,180,110,255 }, {90,160,255,255}, {255,140,55,255}
        };
        int bw3 = (260 - 8) / 3;
        for (int i = 0; i < 3; i++) {
            bool act = (speedIdx == i);
            Color bg  = act ? ColA(SPEED_COLS[i], 0.55f) : ColA(SPEED_COLS[i], 0.10f);
            Color hov = ColA(SPEED_COLS[i], 0.32f);
            Color tc  = act ? WHITE : ColA(SPEED_COLS[i], 0.85f);
            if (Btn(PAD + i*(bw3+4), sY, bw3, 38, SPEED_LABELS[i], bg, hov, tc, 17))  // FIX #1
                speedIdx = i;
            if (act)
                DrawRectangle(PAD + i*(bw3+4)+4, sY+36, bw3-8, 3, SPEED_COLS[i]);
        }
        sY += 46;

        // ─── STATUS ───────────────────────────────────────────────────────
        int stH = 94, stY = scH - stH;
        if (stY < sY + 6) stY = sY + 6;

        // Use theme-aware background: dark overlay in night, light border in day
        Color statBg = nightMode ? ColA({7, 9, 17, 255}, 0.97f)
                                 : ColA({210, 215, 230, 255}, 0.97f);
        DrawRectangle(0, stY, PANEL_W, stH, statBg);
        DrawLine(0, stY, PANEL_W, stY, T.panelEdge);

        DrawText("STATUS", PAD, stY + 8, 14, T.textDim);
        DrawText(statLine1.c_str(), PAD, stY + 26, 18, T.accent);
        auto wl = Wrap(statLine2, PANEL_W - PAD * 2, 15);
        int ly2 = stY + 50;
        for (auto& l : wl) {
            if (ly2 + 17 > stY + stH - 2) break;
            DrawText(l.c_str(), PAD, ly2, 15, T.textMain);  // full opacity in light mode
            ly2 += 18;
        }

        // ─── LIVE STATS ───────────────────────────────────────────────────
        {
            int bw=240, bh=106, bx=scW-bw-14, by=14;
            // In light mode add a stronger border so panel stands out from canvas
            Color statPanelBg = nightMode ? ColA(T.panel, 0.94f)
                                          : ColA({230,234,245,255}, 0.98f);
            Color statPanelEdge = nightMode ? T.panelEdge
                                            : Color{150,165,200,255};
            RRect(bx, by, bw, bh, 0.18f, statPanelBg, statPanelEdge);
            // Top accent bar
            RRect(bx, by, bw, 4, 0.18f, ColA(T.accent, 0.7f));

            DrawText("LIVE STATS", bx+14, by+12, 15, T.textDim);

            int nc = tree.getNodeCount(), ht = tree.getTreeHeight();
            bool bal = (nc <= 1) ? true
                     : (ht <= (int)(1.45f * std::log2((float)nc+1) + 1.6f));

            DrawText(("Nodes:  " + std::to_string(nc)).c_str(), bx+14, by+34, 21, T.textMain);
            DrawText(("Height: " + std::to_string(ht)).c_str(), bx+14, by+59, 21, T.textMain);

            Color bc = bal ? T.success : T.danger;
            DrawText(bal ? "Balanced  [OK]" : "Unbalanced!", bx+14, by+84, 16, bc);
            RRect(bx+14, by+103, bw-28, 3, 0.5f, ColA(bc, 0.5f));
        }

        // ─── TRAVERSAL RESULT ─────────────────────────────────────────────
        if (!travTitle.empty()) {
            int tbH=64, tbY=scH-tbH;
            Color travBg = nightMode ? ColA({8,12,22,252}, 0.97f)
                                     : ColA({215, 220, 235, 252}, 0.97f);
            DrawRectangle(PANEL_W+2, tbY, scW-PANEL_W-2, tbH, travBg);
            DrawLine(PANEL_W+2, tbY, scW, tbY, T.panelEdge);
            DrawText(travTitle.c_str(), PANEL_W+18, tbY+7, 18, T.textDim);

            auto tls = Wrap(travResult, scW-PANEL_W-36, 18);
            int tly = tbY+30;
            for (auto& tl : tls) {
                if (tly+20 > tbY+tbH-2) break;
                DrawText(tl.c_str(), PANEL_W+18, tly, 18, T.accent);
                tly += 21;
            }
        }

        // ─── POPUP ────────────────────────────────────────────────────────
        popup.draw(PANEL_W, scW, T);

        // ─── DEPTH LEGEND ─────────────────────────────────────────────────
        {
            // Draw a subtle pill behind the legend so it's readable on light canvas
            int lx = PANEL_W + 18, ldy = 14;
            int legendW = MeasureText("Depth:", 14) + 8 + 8 * 22 + 10;
            RRect(lx - 6, ldy - 4, legendW, 22, 0.3f,
                  ColA(nightMode ? Color{20,25,40,200} : Color{200,205,220,210}, 1.f));
            DrawText("Depth:", lx, ldy + 2, 14, T.textDim);
            int dotX = lx + MeasureText("Depth:", 14) + 8;
            for (int i = 0; i < 8; i++)
                DrawCircleV({(float)(dotX + i*22 + 8),(float)(ldy+9)}, 6.f, NODE_COLORS[i]);
        }

        // ─── HINTS ────────────────────────────────────────────────────────
        {
            // Pill background so hint text is always readable
            const char* hintTxt = "Scroll: Zoom  |  Right-click+Drag: Pan  |  Double-click: Reset  |  Enter: Insert";
            int htw = MeasureText(hintTxt, 13);
            int hx = PANEL_W + 18, hy = scH - 22;
            RRect(hx - 4, hy - 3, htw + 8, 18, 0.3f,
                  ColA(nightMode ? Color{10,12,22,180} : Color{200,205,220,200}, 1.f));
            DrawText(hintTxt, hx, hy, 13, ColA(T.textDim, 0.85f));
        }

        // ─── NEGATIVE INPUT HINT ──────────────────────────────────────────
        DrawText("Tip: press  -  to toggle negative", PAD, 122+46+4, 13, ColA(T.textDim, 0.6f));

        EndDrawing();
    }

    CloseWindow();
    return 0;
}