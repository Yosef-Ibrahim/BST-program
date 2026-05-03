#pragma once
#include <vector>
#include <tree/BST.h>
#include <ui/ThemeManager.h>

class Renderer {
public:
    virtual void draw(Node<int>* root, const std::vector<int>& searchPath, const Theme& theme) = 0;
};

class TreeRenderer : Renderer {
public:
    void draw(Node<int>* root, const std::vector<int>& searchPath, const Theme& theme) override;

private:
    void drawNodeRecursive(Node<int>* n, int lvl, const std::vector<int>& path, const Theme& theme);
};