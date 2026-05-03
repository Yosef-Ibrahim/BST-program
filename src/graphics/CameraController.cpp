#include <graphics/CameraController.h>

CameraController::CameraController() : panelW(0) {
    cam.target = { 0.0f, 0.0f };
    cam.offset = { 0.0f, 0.0f };
    cam.rotation = 0.0f;
    cam.zoom = 1.0f;
}

void CameraController::init(int screenWidth, int screenHeight, int panelWidth) {
    panelW = panelWidth;

    cam.offset = { (screenWidth + panelW) / 2.0f, screenHeight / 6.0f};
}

void CameraController::update() {
    if (!isMouseOverCanvas(panelW)) return;

    float wheel = GetMouseWheelMove();
    if (wheel != 0.0f) {
        Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), cam);
        cam.offset = GetMousePosition();
        cam.target = mouseWorldPos;

        // Adjust zoom speed
        float zoomIncrease = 0.125f;
        cam.zoom = std::clamp(cam.zoom + wheel * zoomIncrease, 0.25f, 3.0f);
    }

    if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE) || IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        Vector2 delta = GetMouseDelta();
        // Move the target in the opposite direction of the mouse drag
        cam.target.x -= delta.x / cam.zoom;
        cam.target.y -= delta.y / cam.zoom;
    }
}

void CameraController::reset() {
    cam.target = { 0.0f, 0.0f };
    cam.zoom = 1.0f;
}

Camera2D& CameraController::getCamera() { return cam; }

bool CameraController::isMouseOverCanvas(int panelWidth) const {
    return GetMouseX() > panelWidth;
}