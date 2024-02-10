#pragma once

class ZoomMixin {
public:
    ZoomMixin(float zoomX = 1.0f, float zoomY = 10.0f);
    virtual ~ZoomMixin();

    void renderDebugZoomSlider();

    float _zoomX;
    float _zoomY;
};
