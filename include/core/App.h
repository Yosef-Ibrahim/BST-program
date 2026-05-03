#pragma once

#include <raylib.h>
#include <tree/BST.h>
#include <ui/UIManager.h>
#include <ui/ThemeManager.h>
#include <graphics/CameraController.h>
#include <graphics/TreeRenderer.h>

class App {
public:
    App(int width, int height, const char* title);
    ~App();

    void run();

private:
    void update(float dt);
    void draw();

    bool isRunning;

    BST<int> tree;
    ThemeManager themeManager;
    UIManager uiManager;
    CameraController cameraController;
    TreeRenderer treeRenderer;
};