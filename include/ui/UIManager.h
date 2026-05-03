#pragma once
#include <string>
#include <cmath>
#include <functional>
#include <ui/ThemeManager.h>
#include <ui/widgets/IntInput.h>
#include <ui/widgets/Popup.h>
#include <ui/widgets/Button.h>
#include <utility/DrawHelpers.h>

class UIManager {
public:
    UIManager();

    // --- Callbacks ---
    std::function<void(int)> onInsertRequested;
    std::function<void(int)> onRemoveRequested;
    std::function<void()> onClearRequested;
    std::function<void(int)> onPredecessorRequested;
    std::function<void(int)> onSuccessorRequested;
    std::function<void(int)> onTraversalRequested;
    std::function<void(int)> onRandomGenerateRequested;
    std::function<void(const std::string&)> onThemeChangeRequested;

    // --- Core Loop ---
    bool update(float dt);
    
    // draw API
    void draw(const Theme& t, int nodeCount, int treeHeight);

    // API for main app
    void showPopup(const std::string& msg, Color color);
    void setStatus(const std::string& line1, const std::string& line2);
    void setTraversalResult(const std::string& title, const std::string& result);
    void clearTraversalResult();
    float getAnimationSpeedMultiplier() const;

private:
    int panelWidth;
    
    IntInput nodeInput;
    IntInput randomInput;
    Popup popup;

    // UI State
    std::string statLine1;
    std::string statLine2;
    std::string travTitle;
    std::string travResult;

    int speedIdx;
    bool themeMenuOpen;
    int themeIdx;
    int numThemes;
};