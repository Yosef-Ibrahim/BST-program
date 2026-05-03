#include <core/App.h>
#include <set>

App::App(int width, int height, const char* title)
    : window(width, height, title) {

    const int PANEL_W = 350;

    // Camera: canvas center, root near top-sixth of screen
    cameraController.init(width, height, PANEL_W);

    // Insert
    uiManager.onInsertRequested = [this](int value) {
        std::string rotMsg;
        tree.searchPath.clear();
        tree.insert(value, rotMsg);

        Color c = rotMsg.empty()
                  ? themeManager.getCurrentTheme().success
                  : themeManager.getCurrentTheme().accent;
        uiManager.showPopup("Inserted: " + std::to_string(value), c);
        uiManager.setStatus(
            "Inserted " + std::to_string(value),
            rotMsg.empty() ? "No rotation needed" : rotMsg);

        //  Auto-pan camera to the newly inserted node
        float nx = 0.5f, ny = 0.0f;
        if (tree.computeTargetPos(value, nx, ny)) {
            float W      = static_cast<float>(GetScreenWidth());
            float H      = static_cast<float>(GetScreenHeight());
            float panelW = 350.0f;
            float canvasW = (W - panelW) * 2.0f;   // mirrors TreeRenderer spanX
            float spanY   = H / 8.0f;              // mirrors TreeRenderer spanY

            Vector2 worldPos = { nx * canvasW, ny * spanY };
            cameraController.panToWorldPos(worldPos);
        }
    };

    //  Delete 
    uiManager.onRemoveRequested = [this](int value) {
        std::string rotMsg;
        tree.searchPath.clear();
        tree.remove(value, rotMsg);
        uiManager.showPopup("Deleted: " + std::to_string(value),
                             themeManager.getCurrentTheme().error);
        uiManager.setStatus(
            "Deleted " + std::to_string(value),
            rotMsg.empty() ? "" : rotMsg);
    };

    //  Clear 
    uiManager.onClearRequested = [this]() {
        tree.clearTree();
        tree.searchPath.clear();
        uiManager.showPopup("Tree cleared!", themeManager.getCurrentTheme().error);
        uiManager.setStatus("Tree cleared", "");
        uiManager.clearTraversalResult();
    };

    //  Random generate 
    uiManager.onRandomGenerateRequested = [this](int count) {
        tree.clearTree();
        tree.searchPath.clear();
        std::set<int> used;
        for (int i = 0; i < count; ++i) {
            int v = (rand() % 199) - 99;
            if (used.insert(v).second) {
                std::string dummy;
                tree.insert(v, dummy);
            }
        }
        uiManager.showPopup(
            "Generated " + std::to_string(count) + " nodes",
            themeManager.getCurrentTheme().success);
        uiManager.setStatus("Random tree generated", std::to_string(count) + " unique nodes");
        uiManager.clearTraversalResult();
    };

    //  Theme change 
    uiManager.onThemeChangeRequested = [this](const std::string& path) {
        themeManager.loadFromFile(path);
    };

    //  Predecessor (FIX: was never wired up) 
    uiManager.onPredecessorRequested = [this](int value) {
        tree.searchPath.clear();
        int pred = tree.getPredecessor(value);
        if (pred == -1) {
            uiManager.setTraversalResult(
                "Predecessor of " + std::to_string(value),
                "No predecessor found");
            uiManager.showPopup("No predecessor", themeManager.getCurrentTheme().error);
        } else {
            uiManager.setTraversalResult(
                "Predecessor of " + std::to_string(value),
                "Result: " + std::to_string(pred));
            uiManager.showPopup("Predecessor: " + std::to_string(pred),
                                 themeManager.getCurrentTheme().accent);
        }
        uiManager.setStatus("Predecessor search", "Value: " + std::to_string(value));
    };

    //  Successor (FIX: was never wired up) 
    uiManager.onSuccessorRequested = [this](int value) {
        tree.searchPath.clear();
        int succ = tree.getSuccessor(value);
        if (succ == -1) {
            uiManager.setTraversalResult(
                "Successor of " + std::to_string(value),
                "No successor found");
            uiManager.showPopup("No successor", themeManager.getCurrentTheme().error);
        } else {
            uiManager.setTraversalResult(
                "Successor of " + std::to_string(value),
                "Result: " + std::to_string(succ));
            uiManager.showPopup("Successor: " + std::to_string(succ),
                                 themeManager.getCurrentTheme().accent);
        }
        uiManager.setStatus("Successor search", "Value: " + std::to_string(value));
    };

    //  Traversals (FIX: was never wired up) 
    uiManager.onTraversalRequested = [this](int idx) {
        tree.searchPath.clear();
        std::string title, result;
        switch (idx) {
            case 0: title = "Pre-Order (Root → L → R)";  result = tree.getPreOrderString();    break;
            case 1: title = "In-Order (Sorted)";          result = tree.getInOrderString();     break;
            case 2: title = "Post-Order (L → R → Root)"; result = tree.getPostOrderString();   break;
            case 3: title = "Breadth-First (Level)";      result = tree.getBreadthFirstString(); break;
            default: return;
        }
        uiManager.setTraversalResult(title, result);
        uiManager.setStatus(title, "");
    };

    SetTargetFPS(60);
}

App::~App() {}

void App::run() {
    while (!window.shouldClose()) {
        float dt = GetFrameTime();
        update(dt);
        draw();
    }
}

void App::update(float dt) {
    bool uiInteracted = uiManager.update(dt);

    if (!uiInteracted) {
        cameraController.update();
    }

    tree.tickAnimations(dt, uiManager.getAnimationSpeedMultiplier());
}

void App::draw() {
    window.beginDrawing();

    const Theme& theme = themeManager.getCurrentTheme();
    ClearBackground(theme.bg);

    float W      = static_cast<float>(GetScreenWidth());
    float H      = static_cast<float>(GetScreenHeight());
    float panelW = 350.0f;

    // Rebuild camera offset each frame so resizing the window stays correct.
    // The root node (normalised 0,0) appears at the horizontal centre of the
    // canvas and 1/6 down from the top.
    cameraController.getCamera().offset = {
        panelW + (W - panelW) / 2.0f,
        H / 6.0f
    };

    BeginMode2D(cameraController.getCamera());
        treeRenderer.draw(tree.getRoot(), tree.searchPath, theme,
                          static_cast<int>(W), static_cast<int>(H),
                          static_cast<int>(panelW));
    EndMode2D();

    // UI panel (fixed to left side, drawn on top of canvas)
    uiManager.draw(theme, tree.getNodeCount(), tree.getTreeHeight());

    window.endDrawing();
}