#pragma once
#include <vector>
#include <tree/BST.h>
#include <ui/ThemeManager.h>
#include <utility/DrawHelpers.h>
#include <utility/StringUtils.h>
#include <string>

class TreeRenderer {
public:
    void draw(Node<int>* root, const std::vector<int>& searchPath, const Theme& theme, 
                        int screenWidth, int screenHeight, int panelWidth);

private:
    void drawNodeRecursive(Node<int>* node, int lvl, const std::vector<int>& path, const Theme& theme, 
                                     float canvasWidth, float verticalSpacing);
};