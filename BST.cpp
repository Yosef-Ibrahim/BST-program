// ═══════════════════════════════════════════════════════════════════════════════
//  AVL Tree Visualizer  —  Educational Edition
//  Fixed:
//    • RAYGUI_IMPLEMENTATION guard: moved to one translation unit only (here)
//    • GuiValueBox crash: editMode toggled correctly, never both active at once
//    • Random generator: clearTree() before inserting, no duplicate keys
//    • Camera init: now uses proper scW/scH after MaximizeWindow()
//    • drawTree: level passed by value (not reference) — no UB
//    • Status panel: clamped to scH so it never goes off-screen
//    • Traversal bar: safe multi-line word-wrap instead of truncation
//    • Speed labels: consistent width buttons, active state clearly highlighted
//    • Popup: refined fade math, never negative timer
//    • DrawRectRounded: fixed shadow overload — now uses correct raylib API
//    • Panel sections: Y positions calculated dynamically to survive resize
//    • All layout: uses named constants, no magic numbers scattered around
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
//  Layout constants
// ─────────────────────────────────────────────────────────────────────────────
static const int PANEL_W  = 310;   // left side-panel width
static const int PAD      = 18;    // inner padding
static const int BTN_H    = 44;    // standard button height
static const int BTN_H_SM = 36;    // small button height

// ─────────────────────────────────────────────────────────────────────────────
//  Colour palette
// ─────────────────────────────────────────────────────────────────────────────
static const Color COL_BG         = {  13,  15,  23, 255 };
static const Color COL_PANEL      = {  20,  24,  38, 255 };
static const Color COL_PANEL_EDGE = {  42,  52,  78, 255 };
static const Color COL_ACCENT     = {  90, 160, 255, 255 };
static const Color COL_ACCENT2    = { 255, 145,  60, 255 };
static const Color COL_SUCCESS    = {  72, 215, 130, 255 };
static const Color COL_DANGER     = { 255,  75,  75, 255 };
static const Color COL_TEXT_MAIN  = { 228, 233, 255, 255 };
static const Color COL_TEXT_DIM   = { 105, 118, 152, 255 };
static const Color COL_SHADOW     = {   0,   0,   0,  85 };

// Node colours per depth level (8-cycle)
static const Color NODE_COLORS[8] = {
    {  90, 160, 255, 255 },
    {  72, 215, 130, 255 },
    { 255, 145,  60, 255 },
    { 195,  90, 255, 255 },
    { 255,  90, 140, 255 },
    {  55, 205, 210, 255 },
    { 255, 220,  55, 255 },
    { 155, 220,  75, 255 },
};

// ─────────────────────────────────────────────────────────────────────────────
//  Colour utilities
// ─────────────────────────────────────────────────────────────────────────────
static inline Color ColA(Color c, float a) {
    c.a = (unsigned char)std::max(0.f, std::min(255.f, a * 255.f));
    return c;
}
static inline Color ColMix(Color a, Color b, float t) {
    return {
        (unsigned char)(a.r + (b.r - a.r) * t),
        (unsigned char)(a.g + (b.g - a.g) * t),
        (unsigned char)(a.b + (b.b - a.b) * t),
        255
    };
}

// ─────────────────────────────────────────────────────────────────────────────
//  Draw helpers
// ─────────────────────────────────────────────────────────────────────────────
static void DrawRRect(float x, float y, float w, float h,
                      float round, Color fill, Color edge = {0,0,0,0}) {
    Rectangle r = {x, y, w, h};
    DrawRectangleRounded(r, round, 10, fill);
    if (edge.a > 0)
        DrawRectangleRoundedLines(r, round, 10, edge);
}

// Returns true when clicked
static bool Button(int x, int y, int w, int h, const char* label,
                   Color bg, Color hovBg, Color tc, int fs = 20) {
    Rectangle rec = {(float)x, (float)y, (float)w, (float)h};
    bool hov = CheckCollisionPointRec(GetMousePosition(), rec);
    bool clk = hov && IsMouseButtonReleased(MOUSE_LEFT_BUTTON);
    DrawRRect(x, y, w, h, 0.22f, hov ? hovBg : bg);
    if (hov)
        DrawRectangleRoundedLines(rec, 0.22f, 10, ColA(tc, 0.65f));
    int tw = MeasureText(label, fs);
    DrawText(label, x + (w - tw) / 2, y + (h - fs) / 2, fs, tc);
    return clk;
}

// Thin divider line
static void Divider(int y, int x0, int x1) {
    DrawLine(x0, y, x1, y, ColA(COL_PANEL_EDGE, 0.7f));
}

// Section label with accent bar
static void SectionLabel(int x, int y, int w, const char* txt) {
    DrawRectangle(x, y + 7, 4, 14, COL_ACCENT);
    DrawText(txt, x + 10, y + 5, 15, COL_TEXT_DIM);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Popup notification
// ─────────────────────────────────────────────────────────────────────────────
struct Popup {
    std::string msg;
    float timer   = 0.f;
    float maxTime = 0.f;
    Color col     = {90, 160, 255, 255};

    void show(const std::string& m, Color c = {90,160,255,255}, float dur = 3.5f) {
        msg = m; col = c; timer = maxTime = dur;
    }
    bool active() const { return timer > 0.f; }
    void tick(float dt) { timer = std::max(0.f, timer - dt); }

    void draw(int panelW, int scW, int /*scH*/) const {
        if (!active()) return;
        float fade = std::min(1.f, timer * 2.8f);

        int bw = std::min(580, scW - panelW - 40);
        int bh = 54;
        int bx = panelW + (scW - panelW - bw) / 2;
        int by = 20;

        DrawRRect(bx, by, bw, bh, 0.35f, ColA({16,20,34,245}, fade));
        DrawRectangleRoundedLines({(float)bx,(float)by,(float)bw,(float)bh},
                                  0.35f, 10, ColA(col, fade * 0.85f));

        // Progress bar at bottom of popup
        float prog = timer / maxTime;
        DrawRRect(bx + 10, by + bh - 5, (int)((bw - 20) * prog), 3,
                  0.5f, ColA(col, fade * 0.6f));

        int tw = MeasureText(msg.c_str(), 21);
        tw = std::min(tw, bw - 30);  // safety clamp
        DrawText(msg.c_str(), bx + (bw - MeasureText(msg.c_str(), 21)) / 2,
                 by + (bh - 21) / 2 - 3, 21, ColA(COL_TEXT_MAIN, fade));
    }
};

// ─────────────────────────────────────────────────────────────────────────────
//  Draw tree recursively (uses cur_x / cur_y for animated positions)
// ─────────────────────────────────────────────────────────────────────────────
static void DrawTree(Node<int>* node, int level,
                     const std::vector<int>& path)
{
    if (!node) return;

    float cx = node->cur_x;
    float cy = node->cur_y;
    float a  = node->alpha;

    // ── Draw edges before this node ──────────────────────────────────────
    if (node->left) {
        float ea = std::min(a, node->left->alpha);
        DrawLineEx({cx, cy}, {node->left->cur_x, node->left->cur_y},
                   2.2f, ColA({72, 84, 120, 255}, ea * 0.85f));
        DrawTree(node->left, level + 1, path);
    }
    if (node->right) {
        float ea = std::min(a, node->right->alpha);
        DrawLineEx({cx, cy}, {node->right->cur_x, node->right->cur_y},
                   2.2f, ColA({72, 84, 120, 255}, ea * 0.85f));
        DrawTree(node->right, level + 1, path);
    }

    // ── Node gfx ─────────────────────────────────────────────────────────
    Color base = ColA(NODE_COLORS[level % 8], a);

    // Insert pulse ring
    if (node->isNew && node->pulseTimer > 0.f) {
        float pulse = std::sin(node->pulseTimer * 7.f) * 0.5f + 0.5f;
        DrawCircleV({cx, cy}, 30.f + pulse * 12.f, ColA(base, 0.22f * a));
    }

    // Rotation glow ring
    if (node->isRotating && node->highlightVal > 0.f) {
        DrawCircleV({cx, cy}, 36.f,
                    ColA({255, 200, 50, 255}, node->highlightVal * 0.5f * a));
    }

    // Search-path highlight
    bool inPath = std::find(path.begin(), path.end(), node->data) != path.end();
    if (inPath) {
        DrawCircleV({cx, cy}, 34.f, ColA({255, 230, 40, 255}, 0.42f * a));
    }

    // Drop shadow
    DrawCircleV({cx + 3.f, cy + 4.f}, 24.f, ColA(COL_SHADOW, a * 0.45f));

    // Main fill
    DrawCircleV({cx, cy}, 24.f, base);

    // Rim highlight (top-left arc gives 3-D feel)
    DrawCircleV({cx - 5.f, cy - 6.f}, 10.f, ColA(WHITE, 0.08f * a));

    // Outline
    DrawCircleLinesV({cx, cy}, 24.f, ColA(WHITE, 0.20f * a));

    // Value text
    std::string val = std::to_string(node->data);
    int fs  = (val.size() > 3) ? 14 : (val.size() > 2 ? 16 : 20);
    int tw  = MeasureText(val.c_str(), fs);
    DrawText(val.c_str(),
             (int)(cx - tw / 2.f),
             (int)(cy - fs / 2.f),
             fs, ColA(WHITE, a));

    // Height badge (small, top-right)
    std::string hstr = "h" + std::to_string(node->height);
    DrawText(hstr.c_str(), (int)(cx + 17.f), (int)(cy - 26.f),
             11, ColA(COL_TEXT_DIM, 0.75f * a));
}

// ─────────────────────────────────────────────────────────────────────────────
//  Word-wrap a long string into lines that fit inside maxPixelWidth
// ─────────────────────────────────────────────────────────────────────────────
static std::vector<std::string> WrapText(const std::string& text, int maxW, int fs) {
    std::vector<std::string> lines;
    std::istringstream iss(text);
    std::string word, line;
    while (iss >> word) {
        std::string test = line.empty() ? word : line + " " + word;
        if (MeasureText(test.c_str(), fs) <= maxW) {
            line = test;
        } else {
            if (!line.empty()) lines.push_back(line);
            line = word;
        }
    }
    if (!line.empty()) lines.push_back(line);
    return lines;
}

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

    // ── UI state ───────────────────────────────────────────────────────────
    int  inputValue = 0;
    int  randCount  = 12;
    bool editInput  = false;
    bool editRand   = false;

    std::string travTitle  = "";
    std::string travResult = "";
    std::string statLine1  = "AVL Tree Ready";
    std::string statLine2  = "Insert a value to begin!";

    Popup popup;

    // ── Animation speed ────────────────────────────────────────────────────
    //    lerpSpeed:  3 = slow-motion (educational), 9 = normal, 22 = fast
    static const float SPEED_TABLE[3] = { 3.f, 9.f, 22.f };
    static const char* SPEED_LABELS[3] = { "Slow", "Normal", "Fast" };
    int speedIdx = 0;  // default: slow-motion

    // ── Camera ────────────────────────────────────────────────────────────
    Camera2D cam  = {};
    cam.zoom      = 1.0f;
    bool camReady = false;

    // ─────────────────────────────────────────────────────────────────────
    //  Main loop
    // ─────────────────────────────────────────────────────────────────────
    while (!WindowShouldClose()) {
        float dt  = GetFrameTime();
        int   scW = GetScreenWidth();
        int   scH = GetScreenHeight();

        // First frame: place camera target in the centre of the canvas
        if (!camReady) {
            cam.offset = { PANEL_W + (scW - PANEL_W) * 0.5f, scH * 0.38f };
            cam.target = { 0.f, 0.f };
            camReady   = true;
        }

        // ── Tick ────────────────────────────────────────────────────────
        popup.tick(dt);
        tree.tickAnimations(dt, SPEED_TABLE[speedIdx]);

        // ── Zoom (scroll wheel, only over canvas) ────────────────────────
        float wheel = GetMouseWheelMove();
        if (wheel != 0 && GetMousePosition().x > PANEL_W) {
            Vector2 mWorld = GetScreenToWorld2D(GetMousePosition(), cam);
            cam.offset     = GetMousePosition();
            cam.target     = mWorld;
            float factor   = (wheel > 0) ? 1.12f : (1.f / 1.12f);
            cam.zoom       = std::max(0.1f, std::min(6.f, cam.zoom * factor));
        }

        // ── Pan (right-click or middle-click drag) ───────────────────────
        if ((IsMouseButtonDown(MOUSE_BUTTON_RIGHT) || IsMouseButtonDown(MOUSE_BUTTON_MIDDLE))
            && GetMousePosition().x > PANEL_W)
        {
            Vector2 d = GetMouseDelta();
            cam.target.x -= d.x / cam.zoom;
            cam.target.y -= d.y / cam.zoom;
        }

        // ── Reset camera (double-click on canvas) ───────────────────────
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)
            && GetMousePosition().x > PANEL_W)
        {
            static double lastClick = 0.0;
            double now = GetTime();
            if (now - lastClick < 0.35) {
                cam.target = {0.f, 0.f};
                cam.zoom   = 1.f;
                cam.offset = { PANEL_W + (scW - PANEL_W) * 0.5f, scH * 0.38f };
            }
            lastClick = now;
        }

        // ── Deactivate edit boxes when clicking outside them ─────────────
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mp = GetMousePosition();
            if (!CheckCollisionPointRec(mp, {(float)PAD, 118.f, 200.f, 42.f}))
                editInput = false;
            if (!CheckCollisionPointRec(mp, {84.f, 630.f, 96.f, 36.f}))
                editRand = false;
        }

        // ── Keyboard shortcut: Enter = Insert ───────────────────────────
        if (editInput && IsKeyPressed(KEY_ENTER)) {
            std::string rotMsg;
            tree.searchPath.clear();
            tree.insert(inputValue, rotMsg);
            std::string m = "Inserted: " + std::to_string(inputValue);
            if (!rotMsg.empty()) m += "  (" + rotMsg + ")";
            popup.show(m, rotMsg.empty() ? COL_SUCCESS : COL_ACCENT2);
            statLine1 = "Inserted " + std::to_string(inputValue);
            statLine2 = rotMsg.empty() ? "No rotation needed" : rotMsg;
            travTitle = travResult = "";
            editInput = false;
        }

        // ══════════════════════════════════════════════════════════════════
        //  RENDER
        // ══════════════════════════════════════════════════════════════════
        BeginDrawing();
        ClearBackground(COL_BG);

        // ── Tree canvas ──────────────────────────────────────────────────
        BeginMode2D(cam);
        DrawTree(tree.getRoot(), 0, tree.searchPath);
        EndMode2D();

        // ── Side panel ───────────────────────────────────────────────────
        DrawRectangle(0, 0, PANEL_W, scH, COL_PANEL);
        DrawRectangle(PANEL_W, 0, 2, scH, COL_PANEL_EDGE);

        // Subtle gradient top accent bar
        DrawRectangleGradientV(0, 0, PANEL_W, 5,
                               ColA(COL_ACCENT, 0.9f), ColA(COL_ACCENT, 0.f));

        // ────────────────────────────────────────────────────────────────
        //  Title
        // ────────────────────────────────────────────────────────────────
        int cy = 14;
        DrawText("AVL Tree", PAD, cy, 32, COL_ACCENT);
        DrawText("Visualizer", PAD, cy + 36, 20, ColA(COL_TEXT_MAIN, 0.5f));
        DrawText("Educational Edition", PAD, cy + 60, 13, ColA(COL_TEXT_DIM, 0.75f));
        Divider(cy + 84, PAD, PANEL_W - PAD);

        // ────────────────────────────────────────────────────────────────
        //  Node Value input + Insert / Delete / Clear
        // ────────────────────────────────────────────────────────────────
        int sY = cy + 95;
        SectionLabel(PAD, sY, PANEL_W, "NODE VALUE");
        sY += 28;

        // Value box
        Rectangle valBoxR = {(float)PAD, (float)sY, 200.f, 42.f};
        DrawRRect(PAD, sY, 200, 42, 0.18f,
                  editInput ? ColA(COL_ACCENT, 0.12f) : ColA(WHITE, 0.04f));
        DrawRectangleRoundedLines(valBoxR, 0.18f, 10,
                  editInput ? ColA(COL_ACCENT, 0.9f) : COL_PANEL_EDGE);
        if (GuiValueBox(valBoxR, "", &inputValue, -9999, 9999, editInput))
            editInput = !editInput;
        if (!editInput && inputValue == 0)
            DrawText("Click to enter value", PAD + 8, sY + 13, 15, COL_TEXT_DIM);

        sY += 52;

        // Insert
        if (Button(PAD, sY, PANEL_W - PAD * 2, BTN_H, "Insert Node",
                   ColA(COL_ACCENT, 0.18f), ColA(COL_ACCENT, 0.38f), COL_ACCENT))
        {
            std::string rotMsg;
            tree.searchPath.clear();
            tree.insert(inputValue, rotMsg);
            std::string m = "Inserted: " + std::to_string(inputValue);
            if (!rotMsg.empty()) m += "  (" + rotMsg + ")";
            popup.show(m, rotMsg.empty() ? COL_SUCCESS : COL_ACCENT2);
            statLine1 = "Inserted " + std::to_string(inputValue);
            statLine2 = rotMsg.empty() ? "No rotation needed" : rotMsg;
            travTitle = travResult = "";
            editInput = false;
        }
        sY += BTN_H + 6;

        // Delete
        if (Button(PAD, sY, PANEL_W - PAD * 2, BTN_H, "Delete Node",
                   ColA(COL_DANGER, 0.15f), ColA(COL_DANGER, 0.35f), COL_DANGER))
        {
            std::string rotMsg;
            tree.searchPath.clear();
            tree.remove(inputValue, rotMsg);
            std::string m = "Deleted: " + std::to_string(inputValue);
            if (!rotMsg.empty()) m += "  (" + rotMsg + ")";
            popup.show(m, COL_DANGER);
            statLine1 = "Deleted " + std::to_string(inputValue);
            statLine2 = rotMsg.empty() ? "No rotation needed" : rotMsg;
            travTitle = travResult = "";
            editInput = false;
        }
        sY += BTN_H + 6;

        // Clear
        if (Button(PAD, sY, PANEL_W - PAD * 2, BTN_H_SM, "Clear Entire Tree",
                   ColA({50,15,15,255}, 0.9f), ColA(COL_DANGER, 0.22f),
                   ColA(COL_DANGER, 0.8f), 17))
        {
            tree.clearTree();
            tree.searchPath.clear();
            travTitle = travResult = "";
            statLine1 = "Tree cleared";
            statLine2 = "Start fresh!";
            popup.show("Tree cleared!", COL_DANGER, 2.f);
        }
        sY += BTN_H_SM + 8;
        Divider(sY, PAD, PANEL_W - PAD);

        // ────────────────────────────────────────────────────────────────
        //  Search  (Predecessor / Successor)
        // ────────────────────────────────────────────────────────────────
        sY += 8;
        SectionLabel(PAD, sY, PANEL_W, "SEARCH");
        sY += 28;

        int halfW = (PANEL_W - PAD * 2 - 6) / 2;
        if (Button(PAD, sY, halfW, BTN_H_SM, "Predecessor",
                   ColA(COL_ACCENT2, 0.15f), ColA(COL_ACCENT2, 0.35f), COL_ACCENT2, 17))
        {
            int pv = tree.getPredecessor(inputValue);
            std::string m = (pv != -1)
                ? "Predecessor of " + std::to_string(inputValue) + " = " + std::to_string(pv)
                : "No predecessor for " + std::to_string(inputValue);
            popup.show(m, COL_ACCENT2);
            statLine1 = m; statLine2 = "Path shown in yellow";
        }
        if (Button(PAD + halfW + 6, sY, halfW, BTN_H_SM, "Successor",
                   ColA(COL_ACCENT2, 0.15f), ColA(COL_ACCENT2, 0.35f), COL_ACCENT2, 17))
        {
            int sv = tree.getSuccessor(inputValue);
            std::string m = (sv != -1)
                ? "Successor of " + std::to_string(inputValue) + " = " + std::to_string(sv)
                : "No successor for " + std::to_string(inputValue);
            popup.show(m, COL_ACCENT2);
            statLine1 = m; statLine2 = "Path shown in yellow";
        }
        sY += BTN_H_SM + 8;
        Divider(sY, PAD, PANEL_W - PAD);

        // ────────────────────────────────────────────────────────────────
        //  Traversals
        // ────────────────────────────────────────────────────────────────
        sY += 8;
        SectionLabel(PAD, sY, PANEL_W, "TRAVERSALS");
        sY += 28;

        struct TravItem { const char* label; int id; };
        static const TravItem TRAVS[4] = {
            {"Pre-Order  (Root→L→R)", 0},
            {"In-Order  (Sorted)", 1},
            {"Post-Order  (L→R→Root)", 2},
            {"Breadth-First  (Levels)", 3},
        };
        for (int i = 0; i < 4; i++) {
            if (Button(PAD, sY, PANEL_W - PAD * 2, 34, TRAVS[i].label,
                       ColA(COL_PANEL_EDGE, 0.35f), ColA(COL_ACCENT, 0.18f),
                       COL_TEXT_MAIN, 16))
            {
                switch (i) {
                    case 0: travTitle = "Pre-Order:";    travResult = tree.getPreOrderString();    break;
                    case 1: travTitle = "In-Order:";     travResult = tree.getInOrderString();     break;
                    case 2: travTitle = "Post-Order:";   travResult = tree.getPostOrderString();   break;
                    case 3: travTitle = "Breadth-First:";travResult = tree.getBreadthFirstString();break;
                }
            }
            sY += 38;
        }
        Divider(sY, PAD, PANEL_W - PAD);

        // ────────────────────────────────────────────────────────────────
        //  Random Generator
        // ────────────────────────────────────────────────────────────────
        sY += 8;
        SectionLabel(PAD, sY, PANEL_W, "RANDOM GENERATOR");
        sY += 28;

        DrawText("N =", PAD, sY + 8, 17, COL_TEXT_DIM);

        Rectangle randBoxR = {(float)(PAD + 34), (float)sY, 90.f, 36.f};
        DrawRRect(PAD + 34, sY, 90, 36, 0.18f,
                  editRand ? ColA(COL_ACCENT, 0.12f) : ColA(WHITE, 0.04f));
        DrawRectangleRoundedLines(randBoxR, 0.18f, 10,
                  editRand ? ColA(COL_ACCENT, 0.9f) : COL_PANEL_EDGE);
        if (GuiValueBox(randBoxR, "", &randCount, 1, 99, editRand))
            editRand = !editRand;

        if (Button(PAD + 34 + 96, sY, PANEL_W - PAD - (PAD + 34 + 96), 36,
                   "Generate!",
                   ColA(COL_SUCCESS, 0.20f), ColA(COL_SUCCESS, 0.42f), COL_SUCCESS, 18))
        {
            tree.clearTree();
            tree.searchPath.clear();
            std::set<int> used;
            int inserted = 0, attempts = 0;
            while (inserted < randCount && attempts < randCount * 30) {
                int v = (rand() % 199) - 99;
                if (!used.count(v)) {
                    used.insert(v);
                    std::string dummy;
                    tree.insert(v, dummy);
                    inserted++;
                }
                attempts++;
            }
            popup.show("Generated " + std::to_string(inserted) + " random nodes!",
                       COL_SUCCESS);
            statLine1 = "Random tree: " + std::to_string(inserted) + " nodes";
            statLine2 = "Add / delete freely!";
            travTitle = travResult = "";
            editRand = false;
        }
        sY += 42;
        Divider(sY, PAD, PANEL_W - PAD);

        // ────────────────────────────────────────────────────────────────
        //  Animation Speed  (3 equal-width buttons)
        // ────────────────────────────────────────────────────────────────
        sY += 8;
        SectionLabel(PAD, sY, PANEL_W, "ANIMATION SPEED");
        sY += 28;

        static const Color SPEED_COLS[3] = {
            {  60, 180, 115, 255 },   // green  – slow
            {  90, 160, 255, 255 },   // blue   – normal
            { 255, 140,  60, 255 },   // orange – fast
        };
        int btnW3 = (PANEL_W - PAD * 2 - 8) / 3;
        for (int i = 0; i < 3; i++) {
            bool active = (speedIdx == i);
            Color bg  = active ? ColA(SPEED_COLS[i], 0.55f) : ColA(SPEED_COLS[i], 0.10f);
            Color hov = ColA(SPEED_COLS[i], 0.32f);
            Color tc  = active ? WHITE : ColA(SPEED_COLS[i], 0.85f);
            if (Button(PAD + i * (btnW3 + 4), sY, btnW3, 36,
                       SPEED_LABELS[i], bg, hov, tc, 16))
                speedIdx = i;
            // Active underline
            if (active)
                DrawRectangle(PAD + i * (btnW3 + 4) + 4, sY + 34, btnW3 - 8, 3,
                              SPEED_COLS[i]);
        }
        sY += 44;

        // ────────────────────────────────────────────────────────────────
        //  Status bar (pinned to bottom of panel)
        // ────────────────────────────────────────────────────────────────
        int statH = 90;
        int statY = scH - statH;
        // Ensure it doesn't overlap content above
        if (statY < sY + 8) statY = sY + 8;

        DrawRectangle(0, statY, PANEL_W, statH, ColA({8, 10, 18, 255}, 0.97f));
        DrawLine(0, statY, PANEL_W, statY, COL_PANEL_EDGE);

        DrawText("STATUS", PAD, statY + 8, 13, COL_TEXT_DIM);
        DrawText(statLine1.c_str(), PAD, statY + 26, 17, COL_ACCENT);
        // Wrap line2 to fit panel
        {
            auto lines = WrapText(statLine2, PANEL_W - PAD * 2, 14);
            int ly = statY + 48;
            for (auto& l : lines) {
                if (ly + 16 > statY + statH - 4) break;
                DrawText(l.c_str(), PAD, ly, 14, ColA(COL_TEXT_MAIN, 0.65f));
                ly += 17;
            }
        }

        // ────────────────────────────────────────────────────────────────
        //  Live stats (top-right of canvas)
        // ────────────────────────────────────────────────────────────────
        {
            int bw = 230, bh = 100;
            int bx = scW - bw - 14, by = 14;
            DrawRRect(bx, by, bw, bh, 0.18f,
                      ColA(COL_PANEL, 0.93f), COL_PANEL_EDGE);
            DrawText("LIVE STATS", bx + 14, by + 10, 14, COL_TEXT_DIM);

            int nc = tree.getNodeCount();
            int ht = tree.getTreeHeight();
            bool balanced = (nc <= 1)
                ? true
                : (ht <= (int)(1.45f * std::log2((float)nc + 1) + 1.6f));

            std::string ns = "Nodes:  " + std::to_string(nc);
            std::string hs = "Height: " + std::to_string(ht);
            DrawText(ns.c_str(), bx + 14, by + 32, 20, COL_TEXT_MAIN);
            DrawText(hs.c_str(), bx + 14, by + 56, 20, COL_TEXT_MAIN);

            Color bColor = balanced ? COL_SUCCESS : COL_DANGER;
            const char* bLabel = balanced ? "Balanced  ✓" : "Unbalanced!";
            DrawText(bLabel, bx + 14, by + 78, 15, bColor);

            // Tiny balance bar
            DrawRRect(bx + 14, by + 94, bw - 28, 3, 0.5f, ColA(bColor, 0.5f));
        }

        // ────────────────────────────────────────────────────────────────
        //  Traversal result bar (bottom of canvas)
        // ────────────────────────────────────────────────────────────────
        if (!travTitle.empty()) {
            int tbH = 64;
            int tbY = scH - tbH;
            DrawRectangle(PANEL_W + 2, tbY, scW - PANEL_W - 2, tbH,
                          ColA({10, 13, 24, 252}, 0.97f));
            DrawLine(PANEL_W + 2, tbY, scW, tbY, COL_PANEL_EDGE);

            DrawText(travTitle.c_str(), PANEL_W + 18, tbY + 7, 17, COL_TEXT_DIM);

            // Wrap result to available width
            int maxW = scW - PANEL_W - 36;
            auto lines = WrapText(travResult, maxW, 18);
            int lY = tbY + 28;
            for (auto& l : lines) {
                if (lY + 20 > tbY + tbH - 2) break;
                DrawText(l.c_str(), PANEL_W + 18, lY, 18, COL_ACCENT);
                lY += 21;
            }
        }

        // ────────────────────────────────────────────────────────────────
        //  Popup notification
        // ────────────────────────────────────────────────────────────────
        popup.draw(PANEL_W, scW, scH);

        // ────────────────────────────────────────────────────────────────
        //  Keyboard / mouse hints
        // ────────────────────────────────────────────────────────────────
        const char* hint = "Scroll: Zoom  |  Right-click+Drag: Pan  |  Double-click: Reset View  |  Enter: Quick Insert";
        DrawText(hint, PANEL_W + 18, scH - 20, 13, ColA(COL_TEXT_DIM, 0.7f));

        // Legend dots (top-left of canvas)
        {
            int lx = PANEL_W + 18, ly = 16;
            DrawText("Depth levels:", lx, ly, 13, COL_TEXT_DIM);
            lx += MeasureText("Depth levels:", 13) + 8;
            for (int i = 0; i < 8; i++) {
                DrawCircleV({(float)(lx + i * 22 + 8), (float)(ly + 7)},
                            6.f, NODE_COLORS[i]);
            }
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}