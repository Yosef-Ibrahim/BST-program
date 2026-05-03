#pragma once
#include <raylib.h>

class CameraController {
public:
    CameraController();

    void init(int screenWidth, int screenHeight, int panelWidth);
    void update();
    void reset();

    Camera2D& getCamera();
    bool isMouseOverCanvas(int panelWidth) const;

private:
    Camera2D cam;
    int panelW;
    double lastClickTime;
};