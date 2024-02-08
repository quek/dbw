#pragma once

class ZoomMixin {
protected:
    ZoomMixin(float zoomX, float zoomY);
    void renderDebugZoomSlider();
    float _zoomX = 4.0f;
    float _zoomY = 0.5f;
};
