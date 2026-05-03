#pragma once
#include <raylib.h>
#include <algorithm>
#include <cmath>

class CameraController {
public:
    CameraController();

    void init(int screenWidth, int screenHeight, int panelWidth);

    void update();

    void panToWorldPos(Vector2 worldTarget);

    void reset();

    Camera2D& getCamera();

private:
    Camera2D cam;
    int      panelW;

    double lastClickTime;
    float  doubleClickThreshold;

    bool    isFollowing;
    Vector2 followTarget;
    float   followSpeed;

    float   targetZoom;

    bool isMouseOverCanvas(int panelWidth) const;
};