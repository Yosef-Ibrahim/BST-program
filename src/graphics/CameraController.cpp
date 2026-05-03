#include <graphics/CameraController.h>

CameraController::CameraController()
    : panelW(0),
      lastClickTime(0.0),
      doubleClickThreshold(0.35f),
      isFollowing(false),
      followTarget({ 0.0f, 0.0f }),
      followSpeed(6.0f),
      targetZoom(1.0f)
{
    cam.target   = { 0.0f, 0.0f };
    cam.offset   = { 0.0f, 0.0f };
    cam.rotation = 0.0f;
    cam.zoom     = 1.0f;
}

void CameraController::init(int screenWidth, int screenHeight, int panelWidth) {
    panelW = panelWidth;
    cam.offset = {
        panelW + (screenWidth - panelW) / 2.0f,
        screenHeight / 6.0f
    };
}

void CameraController::panToWorldPos(Vector2 worldTarget) {
    followTarget = worldTarget;
    isFollowing  = true;
}

void CameraController::update() {
    float dt = GetFrameTime();

    // -- Smooth follow --------------------------------------------------------
    if (isFollowing) {
        float dx   = followTarget.x - cam.target.x;
        float dy   = followTarget.y - cam.target.y;
        float dist = std::sqrt(dx*dx + dy*dy);

        if (dist < 1.0f) {
            cam.target  = followTarget;
            isFollowing = false;
        } else {
            float factor = std::clamp(followSpeed * dt, 0.0f, 1.0f);
            cam.target.x += dx * factor;
            cam.target.y += dy * factor;
        }
    }

    // -- Smooth zoom lerp (every frame) ---------------------------------------
    // cam.zoom chases targetZoom with exponential ease-out.
    cam.zoom += (targetZoom - cam.zoom) * std::clamp(dt * 14.0f, 0.0f, 1.0f);

    if (!isMouseOverCanvas(panelW)) return;

    // -- Double-click to reset ------------------------------------------------
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        double now = GetTime();
        if ((now - lastClickTime) <= doubleClickThreshold) {
            reset();
            lastClickTime = 0.0;
        } else {
            lastClickTime = now;
        }
    }

    // -- Scroll-wheel: update targetZoom, keep cursor world-point pinned -------
    float wheel = GetMouseWheelMove();
    if (wheel != 0.0f) {
        // Capture the world point under the cursor BEFORE zoom changes.
        Vector2 mouseScreen = GetMousePosition();
        Vector2 mouseWorld  = GetScreenToWorld2D(mouseScreen, cam);

        float newZoom = std::clamp(targetZoom + wheel * 0.2f, 0.1f, 5.0f);
        targetZoom    = newZoom;

        cam.target.x = mouseWorld.x - (mouseScreen.x - cam.offset.x) / targetZoom;
        cam.target.y = mouseWorld.y - (mouseScreen.y - cam.offset.y) / targetZoom;
    }

    // -- Middle / right-click pan (cancels auto-follow) -----------------------
    if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE) || IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        isFollowing = false;
        Vector2 delta = GetMouseDelta();
        cam.target.x -= delta.x / cam.zoom;
        cam.target.y -= delta.y / cam.zoom;
    }
}

void CameraController::reset() {
    cam.target = { 0.0f, 0.0f };
    cam.zoom   = 1.0f;
    targetZoom = 1.0f;
    isFollowing = false;
}

Camera2D& CameraController::getCamera() { return cam; }

bool CameraController::isMouseOverCanvas(int panelWidth) const {
    return GetMouseX() > panelWidth;
}