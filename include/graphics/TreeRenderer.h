#pragma once
#include <vector>
#include <tree/BST.h>
#include <ui/ThemeManager.h>

class TreeRenderer {
public:
    void draw(Node<int>* root, const std::vector<int>& searchPath, const Theme& theme);

private:
    void drawNodeRecursive(Node<int>* n, int lvl, const std::vector<int>& path, const Theme& theme);
};