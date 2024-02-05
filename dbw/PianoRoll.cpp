#include "PianoRoll.h"
#include <memory>
#include "Grid.h"
#include "Midi.h"

std::unique_ptr<PianoRoll> gPianoRoll = nullptr;

constexpr float BEAT_WIDTH = 10.0f;
constexpr float KEYBOARD_WIDTH = 30.0f;
constexpr float TIMELINE_HEIGHT = 15.0f;
constexpr float TIMELINE_START_OFFSET = 10.0f;
constexpr float GRID_SKIP_WIDTH = 20.0f;
float KEY_HEIGHT = 50.0f;

ImU32 BAR_LINE_COLOR = IM_COL32(0x88, 0x88, 0x88, 0x88);
ImU32 BEAT_LINE_COLOR = IM_COL32(0x55, 0x55, 0x55, 0x88);
ImU32 BEAT16TH_LINE_COLOR = IM_COL32(0x33, 0x33, 0x33, 0x88);
ImU32 BACKGROUD_WHITE_KEY_COLOR = IM_COL32(0x22, 0x22, 0x22, 0x88);
ImU32 BACKGROUD_BLACK_KEY_COLOR = IM_COL32(0x00, 0x00, 0x00, 0x88);

ImVec2 operator+(const ImVec2& lhs, const ImVec2& rhs) {
    return ImVec2(lhs.x + rhs.x, lhs.y + rhs.y);
}

ImVec2& operator+=(ImVec2& lhs, const ImVec2& rhs) {
    lhs.x += rhs.x;
    lhs.y += rhs.y;
    return lhs;
}

ImVec2 operator-(const ImVec2& lhs, const ImVec2& rhs) {
    return ImVec2(lhs.x - rhs.x, lhs.y - rhs.y);
}

ImVec2 operator*(const ImVec2& lhs, const ImVec2& rhs) {
    return ImVec2(lhs.x * rhs.x, lhs.y * rhs.y);
}

bool operator==(const ImVec2& lhs, const ImVec2& rhs) {
    return lhs.x == rhs.x && lhs.y == rhs.y;
}

bool operator!=(const ImVec2& lhs, const ImVec2& rhs) {
    return !(lhs == rhs);
}

bool operator<(const ImVec2& lhs, const ImVec2& rhs) {
    return lhs.x < rhs.x && lhs.y < rhs.y;
}

bool operator<=(const ImVec2& lhs, const ImVec2& rhs) {
    return lhs.x <= rhs.x && lhs.y <= rhs.y;
}

bool operator>(const ImVec2& lhs, const ImVec2& rhs) {
    return lhs.x > rhs.x && lhs.y > rhs.y;
}

bool operator>=(const ImVec2& lhs, const ImVec2& rhs) {
    return lhs.x >= rhs.x && lhs.y >= rhs.y;
}

PianoRoll::PianoRoll() {
    _grid = gGrids[1].get();
}

void PianoRoll::render() {
    if (!_show) return;

    if (ImGui::Begin("Piano Roll", &_show)) {
        int gridIndex = 0;
        for (; gridIndex < gGrids.size(); ++gridIndex) {
            if (gGrids[gridIndex].get() == _grid) {
                break;
            }
        }
        ImGui::SetNextItemWidth(ImGui::GetFontSize() * 3.5f);
        auto pred = [](void* x, int i) {
            return (*(std::vector<std::unique_ptr<Grid>>*)x)[i]->_name.c_str();
        };
        if (ImGui::Combo("Grid", &gridIndex, pred, &gGrids, static_cast<int>(gGrids.size()))) {
            _grid = gGrids[gridIndex].get();
        }
        ImGui::SameLine();
        ImGui::Checkbox("Snap", &_snap);

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
        {
            if (ImGui::BeginChild("##Piano Roll Canvas",
                                  ImVec2(0, 0),
                                  ImGuiChildFlags_None,
                                  ImGuiWindowFlags_HorizontalScrollbar)) {
                ImVec2 windowPos = ImGui::GetWindowPos();
                ImVec2 clipRectMin = windowPos + ImVec2(KEYBOARD_WIDTH, TIMELINE_HEIGHT);
                ImVec2 clipRectMax = ImVec2(clipRectMin.x + ImGui::GetWindowWidth(), clipRectMin.y + ImGui::GetWindowHeight());
                ImGui::PushClipRect(clipRectMin, clipRectMax, true);
                {
                    renderGrid();
                    renderBackgroud();
                    handleCanvas();
                }
                ImGui::PopClipRect();
                renderTimeline();
                renderKeyboard();
            }
            ImGui::EndChild();
        }
        ImGui::PopStyleVar();

        // TODO マウスホイールとかでスクロールするようにする
        ImGui::SetNextItemWidth(ImGui::GetFontSize() * 5);
        ImGui::DragFloat("Zoom X", &_zoomX, 0.01f, 0.001f);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ImGui::GetFontSize() * 5);
        ImGui::DragFloat("Zoom Y", &_zoomY, 0.01f, 0.001f);
    }
    ImGui::End();
}

void PianoRoll::edit(Clip* clip) {
    _clip = clip;
    _show = true;
}

int PianoRoll::maxBar() {
    // TODO
    return 50;
}

void PianoRoll::renderBackgroud() {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 windowPos = ImGui::GetWindowPos();
    float scrollY = ImGui::GetScrollY();
    float windowWidth = ImGui::GetWindowWidth();
    float x1 = windowPos.x + KEYBOARD_WIDTH;
    float x2 = x1 + windowWidth + KEYBOARD_WIDTH;
    for (int i = 127; i >= 0; --i) {
        auto mod = i % 12;
        ImU32 color = BACKGROUD_WHITE_KEY_COLOR;
        if (mod == 1 || mod == 3 || mod == 6 || mod == 8 || mod == 10) {
            color = BACKGROUD_BLACK_KEY_COLOR;
        }
        float y1 = KEY_HEIGHT * i * _zoomY + windowPos.y - scrollY + TIMELINE_HEIGHT;
        float y2 = KEY_HEIGHT * (i + 1) * _zoomY + windowPos.y - scrollY + TIMELINE_HEIGHT;
        drawList->AddRectFilled(ImVec2(x1, y1), ImVec2(x2, y2), color);
    }
}

void PianoRoll::renderGrid() {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 windowPos = ImGui::GetWindowPos();
    float windowHeight = ImGui::GetWindowHeight();
    float y1 = windowPos.y;
    float y2 = windowPos.y + windowHeight;
    float lastBarX = -GRID_SKIP_WIDTH;
    float scrollX = ImGui::GetScrollX();
    for (int bar = 0; bar < maxBar(); ++bar) {
        float barX = (BEAT_WIDTH * 4 * bar * _zoomX) + windowPos.x + TIMELINE_START_OFFSET + KEYBOARD_WIDTH - scrollX;
        if (barX - lastBarX < GRID_SKIP_WIDTH) {
            continue;
        }
        drawList->AddLine(ImVec2(barX, y1), ImVec2(barX, y2), BAR_LINE_COLOR);

        renderGridBeat16th(drawList, barX, y1, y2);

        if (BEAT_WIDTH * _zoomX >= GRID_SKIP_WIDTH) {
            for (int beat = 1; beat < 4; ++beat) {
                float beatX = barX + (BEAT_WIDTH * beat * _zoomX);
                drawList->AddLine(ImVec2(beatX, y1), ImVec2(beatX, y2), BEAT_LINE_COLOR);

                renderGridBeat16th(drawList, beatX, y1, y2);
            }
        } else if (BEAT_WIDTH * 2 * _zoomX >= GRID_SKIP_WIDTH) {
            float beatX = barX + (BEAT_WIDTH * 2 * _zoomX);
            drawList->AddLine(ImVec2(beatX, y1), ImVec2(beatX, y2), BEAT_LINE_COLOR);
        }

        lastBarX = barX;
    }
}

void PianoRoll::renderGridBeat16th(ImDrawList* drawList, float beatX, float y1, float y2) {
    if (BEAT_WIDTH / 4 * _zoomX >= GRID_SKIP_WIDTH) {
        for (int beat16th = 1; beat16th < 4; ++beat16th) {
            float beat16thX = beatX + (BEAT_WIDTH / 4 * beat16th * _zoomX);
            drawList->AddLine(ImVec2(beat16thX, y1), ImVec2(beat16thX, y2), BEAT16TH_LINE_COLOR, 0.5f);
        }
    }
}

void PianoRoll::renderKeyboard() {
    ImGui::BeginGroup();
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
    float scrollX = ImGui::GetScrollX();
    for (int i = 127; i >= 0; --i) {
        auto note = gMidiNumToSym[i];
        auto mod = i % 12;
        if (mod == 1 || mod == 3 || mod == 6 || mod == 8 || mod == 10) {
            ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32_BLACK);
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32_WHITE);
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32_WHITE);
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32_BLACK);
        }
        ImGui::SetCursorPos(ImVec2(scrollX, KEY_HEIGHT * (127 - i) * _zoomY + TIMELINE_HEIGHT));
        if (ImGui::Button(note.c_str(), ImVec2(KEYBOARD_WIDTH, KEY_HEIGHT * _zoomY))) {
            // ...
        }
        ImGui::PopStyleColor(2);
    }
    ImGui::PopStyleVar(2);
    ImGui::EndGroup();
}

void PianoRoll::renderTimeline() {
    float leftPadding = 2.0f;
    float scrollY = ImGui::GetScrollY();
    float lastX = -GRID_SKIP_WIDTH;
    for (int i = 0; i < maxBar(); ++i) {
        float x = (BEAT_WIDTH * 4 * i * _zoomX) + KEYBOARD_WIDTH + TIMELINE_START_OFFSET;
        if (x - lastX < GRID_SKIP_WIDTH) {
            continue;
        } else {
            lastX = x;
        }
        float y = scrollY;
        ImVec2 pos = ImVec2(x + leftPadding, y);
        ImGui::SetCursorPos(pos);
        ImGui::Text(std::to_string(i + 1).c_str());
    }
}

void PianoRoll::handleCanvas() {
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 windowPos = ImGui::GetWindowPos();
    ImVec2 windowSize = ImGui::GetWindowSize();
    float scrollX = ImGui::GetScrollX();
    float scrollY = ImGui::GetScrollY();
    ImGui::SetCursorPos(ImVec2(KEYBOARD_WIDTH + scrollX + 10.0f, 100 + scrollY));
    ImGui::BeginGroup();
    ImGui::Text("Mouser %f %f", io.MousePos.x, io.MousePos.y);
    ImGui::Text("GetWindowPos%f %f", windowPos.x, windowPos.y);
    ImGui::Text("GetWindowSize %f %f", windowSize.x, windowSize.y);
    ImGui::Text("GetScrollX/Y %f %f", scrollX, scrollY);
    ImGui::Text("Debug foo: %d", _state.foo);
    ImGui::EndGroup();

    if (windowPos <= io.MousePos && io.MousePos < windowPos + windowSize) {
        if (_state.foo == 0) {
            _state.foo = 1;
        }
        if (io.MouseDoubleClicked[0]) {
            _state.foo = 2;
        }
    } else {
        _state.foo = 0;
    }

    ImGui::SetCursorPos(ImVec2(1024, 1024));
    ImGui::Text("Debug foo: %d", _state.foo);
}
