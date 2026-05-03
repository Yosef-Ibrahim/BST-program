#pragma once
#include <raylib.h>
#include <string>
#include <ui/ThemeManager.h>
#include <tree/BST.h>

class UIManager {
public:
    UIManager();
    
    // Returns true if the user interacted with the UI (useful for blocking camera pan)
    bool update(float dt, BST<int>& tree, ThemeManager& themeManager);
    void draw(const ThemeManager& themeManager);

private:
    // UI Widgets would go here
    // IntInput valInput;
    // Popup popup;
    
    std::string statLine1;
    std::string statLine2;
};