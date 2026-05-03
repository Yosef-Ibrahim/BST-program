#pragma once
#include <core/Window.h>
#include <tree/BST.h>
#include <graphics/CameraController.h>
#include <graphics/TreeRenderer.h>
#include <ui/UIManager.h>
#include <ui/ThemeManager.h>
#include <set>

class App {
public:
    App(int width, int height, const char* title);
    ~App();
    
    void run();

private:
    void update(float dt);
    void draw();

    Window window;
    BST<int> tree;
    CameraController cameraController;
    TreeRenderer treeRenderer;
    UIManager uiManager;
    ThemeManager themeManager;
};