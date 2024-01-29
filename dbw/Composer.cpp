#include "Composer.h"
#include <cstdlib>
#include <map>
#include <ranges>
#include <sstream>
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "AudioEngine.h"
#include "Column.h"
#include "Command.h"
#include "ErrorWindow.h"
#include "GuiUtil.h"
#include "Line.h"
#include "Project.h"
#include "logger.h"
#include "Track.h"
#include "util.h"

Composer::Composer(AudioEngine* audioEngine) :
    _audioEngine(audioEngine),
    _commandManager(this),
    _pluginManager(this),
    _project(std::make_unique<Project>("noname", this)),
    _masterTrack(std::make_unique<MasterTrack>(this)) {
    addTrack();
}

void Composer::process(float* /* in */, float* out, unsigned long framesPerBuffer, int64_t steadyTime) {
    _masterTrack->_processBuffer.clear();
    _masterTrack->_processBuffer.ensure(framesPerBuffer, 2);

    if (_playing) {
        _nextPlayPosition = _playPosition.nextPlayPosition(_audioEngine->_sampleRate, framesPerBuffer, _bpm, _lpb, &_samplePerDelay);
    }

    _processBuffer.clear();
    _processBuffer.ensure(framesPerBuffer, 2);

    _masterTrack->_processBuffer.ensure(framesPerBuffer, 2);
    _masterTrack->_processBuffer._in.zero();
    for (auto iter = _tracks.begin(); iter != _tracks.end(); ++iter) {
        auto& track = *iter;
        track->_processBuffer.clear();
        track->_processBuffer.ensure(framesPerBuffer, 2);
        track->process(steadyTime);
        _masterTrack->_processBuffer._in.add(track->_processBuffer._out);
    }

    _masterTrack->process(steadyTime);
    _masterTrack->_processBuffer._out.copyTo(out, framesPerBuffer, 2);

    if (_playing) {
        _playPosition = _nextPlayPosition;
    }
    if (_looping) {
        if (_playPosition >= _loopEndPosition) {
            _playPosition = _loopStartPosition;
        }
    }
    if (_playPosition._line > _maxLine) {
        _playPosition._line = 0;
        _playPosition._delay = 0;
    }
}

void Composer::scanPlugin() {
    _pluginManager.scan();
}

void Composer::setStatusMessage(std::string message) {
    _statusMessage = message;
}

void Composer::changeMaxLine() {
    for (auto track = _tracks.begin(); track != _tracks.end(); ++track) {
        (*track)->changeMaxLine(_maxLine);
    }
}

class AddColumnCommand : public Command {
public:
    AddColumnCommand(Track* track) : _track(track) {
        for (auto line = _track->_lines.begin(); line != _track->_lines.end(); ++line) {
            _columns.push_back(std::make_unique<Column>((*line).get()));
        }
    }
    void execute(Composer* composer) override {
        std::lock_guard<std::mutex> lock(composer->_audioEngine->mtx);
        for (auto [line, column] : std::views::zip(_track->_lines, _columns)) {
            line->_columns.push_back(std::move(column));
        }
        _track->_ncolumns++;
        _track->_lastKeys.push_back(0);
    }
    void undo(Composer* composer) override {
        std::lock_guard<std::mutex> lock(composer->_audioEngine->mtx);
        for (auto [line, column] : std::views::zip(_track->_lines, _columns)) {
            column = std::move(line->_columns.back());
            line->_columns.pop_back();
        }
        _track->_ncolumns--;
        _track->_lastKeys.pop_back();
    }
    Track* _track;
    std::vector<std::unique_ptr<Column>> _columns;
};

class DeleteColumnCommand : public Command {
public:
    DeleteColumnCommand(Track* track) : _track(track) {
        for (auto i = 0; i < _track->_composer->_maxLine; ++i) {
            _columns.push_back(std::make_unique<Column>(nullptr));
        }
    }
    void execute(Composer* composer) override {
        std::lock_guard<std::mutex> lock(composer->_audioEngine->mtx);
        for (auto [line, column] : std::views::zip( _track->_lines, _columns)) {
            column = std::move(line->_columns.back());
            line->_columns.pop_back();
        }
        _track->_ncolumns--;
        _track->_lastKeys.pop_back();
    }
    void undo(Composer* composer) override {
        std::lock_guard<std::mutex> lock(composer->_audioEngine->mtx);
        for (auto [line, column] : std::views::zip( _track->_lines, _columns)) {
            line->_columns.push_back(std::move(column));
        }
        _columns.clear();
        _track->_ncolumns++;
        _track->_lastKeys.push_back(0);
    }
    Track* _track;
    std::vector<std::unique_ptr<Column>> _columns;
};



void Composer::render() {
    _commandManager.run();

    ImGui::Begin("main window");   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)

    if (_playing) {
        ImGui::PushStyleColor(ImGuiCol_Button, COLOR_BUTTON_ON);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, COLOR_BUTTON_ON_HOVERED);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, COLOR_BUTTON_ON_ACTIVE);
        if (ImGui::Button("Play")) {
            stop();
        }
        ImGui::PopStyleColor(3);
    } else {
        if (ImGui::Button("Play")) {
            play();
        }
    }
    ImGui::SameLine();
    if (_looping) {
        ImGui::PushStyleColor(ImGuiCol_Button, COLOR_BUTTON_ON);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, COLOR_BUTTON_ON_HOVERED);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, COLOR_BUTTON_ON_ACTIVE);
        if (ImGui::Button("Loop")) {
            _looping = false;
            gErrorWindow->show("Stop loop...\nHello World!\naaaaaaaaaaaaaaaaaaaaaaabbbbbbbbbbbbbbbbbbbbbbbbbbbbbbcccccccccccccccccccccccccccceeeeeeeeeeeeeeeeeeeeeeeee  dflaksdjf  lskdfjsda  as;ldkfjasdklf alfs;kjsadlkfa asd;flkjsadl;kfjasd;l asdlkfjsadlkf sadfklsdfjla asdfasdf");
        }
        ImGui::PopStyleColor(3);
    } else {
        if (ImGui::Button("Loop")) {
            _looping = true;
            gErrorWindow->show("Start loop...");
        }
    }
    ImGui::SameLine();
    if (_scrollLock) {
        ImGui::PushStyleColor(ImGuiCol_Button, COLOR_BUTTON_ON);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, COLOR_BUTTON_ON_HOVERED);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, COLOR_BUTTON_ON_ACTIVE);
        if (ImGui::Button("ScLk")) {
            _scrollLock = false;
        }
        ImGui::PopStyleColor(3);
    } else {
        if (ImGui::Button("ScLk")) {
            _scrollLock = true;
        }
    }
    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetFontSize() * 4);
    if (ImGui::DragInt("Lines", &_maxLine, 1.0f, 1, 0x40 * 1000)) {
        changeMaxLine();
    }
    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetFontSize() * 5);
    ImGui::DragFloat("BPM", &_bpm, 1.0f, 0.0f, 999.0f, "%.02f");
    ImGui::SameLine();
    if (ImGui::Button("Undo")) {
        _commandManager.undo();
    }
    ImGui::SameLine();
    if (ImGui::Button("Redo")) {
        _commandManager.redo();
    }
    ImGui::SameLine();
    if (ImGui::Button("Scan Plugin")) {
        scanPlugin();
    }
    ImGui::SameLine();
    if (ImGui::Button("Open")) {
        _project->open();
    }
    ImGui::SameLine();
    if (ImGui::Button("Save")) {
        _project->save();
        setStatusMessage(std::string("Project is saved ") + yyyyMmDdHhMmSs());
    }

    ImGuiTableFlags flags = ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;
    // When using ScrollX or ScrollY we need to specify a size for our table container!
    // Otherwise by default the table will fit all available space, like a BeginChild() call.
    ImVec2 outer_size = ImVec2(0.0f, TEXT_BASE_HEIGHT * 20);
    std::vector<float> columnWidths;
    // 2 is timeline and master track.
    if (ImGui::BeginTable("tracks", 2 + static_cast<int>(_tracks.size()), flags, outer_size)) {
        ImGui::TableSetupScrollFreeze(1, 1);
        // Make the first column not hideable to match our use of TableSetupScrollFreeze()
        ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_NoResize);
        for (auto i = 0; i < _tracks.size(); ++i) {
            ImGui::TableSetupColumn(_tracks[i]->_name.c_str(), ImGuiTableColumnFlags_NoResize);
        }
        ImGui::TableSetupColumn(_masterTrack->_name.c_str(), ImGuiTableColumnFlags_NoResize);

        //ImGui::TableHeadersRow();
        ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
        ImGui::TableSetColumnIndex(0);
        ImGui::TableHeader("#");
        for (auto i = 0; i < _tracks.size(); ++i) {
            ImGui::TableSetColumnIndex(i + 1);
            ImGui::PushID(i);
            auto name = _tracks[i]->_name.c_str();
            ImGui::TableHeader(name);
            ImGui::SameLine(ImGui::CalcTextSize(name).x + ImGui::GetStyle().ItemSpacing.x);
            if (ImGui::Button("+")) {
                _commandManager.executeCommand(new AddColumnCommand(_tracks[i].get()));
            }
            if (_tracks[i]->_ncolumns > 1) {
                ImGui::SameLine();
                if (ImGui::Button("-")) {
                    _commandManager.executeCommand(new DeleteColumnCommand(_tracks[i].get()));
                }
            }
            ImGui::PopID();
        }
        ImGui::TableSetColumnIndex(static_cast<int>(_tracks.size() + 1));
        ImGui::PushID(_masterTrack.get());
        ImGui::TableHeader(_masterTrack->_name.c_str());
        ImGui::PopID();

        for (int line = 0; line < _maxLine; line++) {
            ImGui::TableNextRow();
            if (line == _playPosition._line) {
                ImU32 cell_bg_color = ImGui::GetColorU32(COLOR_PALY_LINE);
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, cell_bg_color);
                if (!_scrollLock && _playing) {
                    ImGui::SetScrollHereY(0.5f); // 0.0f:top, 0.5f:center, 1.0f:bottom
                }
            }
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%02X", line);
            if (line == 0) {
                columnWidths.push_back(ImGui::GetContentRegionAvail().x);
            }
            for (auto i = 0; i < _tracks.size(); ++i) {
                auto track = _tracks[i].get();
                ImGui::TableSetColumnIndex(i + 1);
                track->renderLine(line);
                if (line == 0) {
                    columnWidths.push_back(ImGui::GetContentRegionAvail().x);
                }
            }

            ImGui::TableSetColumnIndex(static_cast<int>(_tracks.size() + 1));
            _masterTrack->renderLine(line);
            if (line == 0) {
                columnWidths.push_back(ImGui::GetContentRegionAvail().x);
            }
        }
        ImGui::EndTable();
    }

    if (ImGui::BeginTable("racks", 2 + static_cast<int>(_tracks.size()), flags, ImVec2(0.0f, TEXT_BASE_HEIGHT * 10))) {
        ImGui::TableSetupScrollFreeze(1, 1);
        ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_WidthFixed, columnWidths[0]);
        for (size_t i = 0; i < _tracks.size(); ++i) {
            ImGui::TableSetupColumn(_tracks[i]->_name.c_str(), ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_WidthFixed, columnWidths[i + 1]);
        }
        ImGui::TableSetupColumn(_masterTrack->_name.c_str(), ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_WidthFixed, columnWidths[_tracks.size() + 1]);
        ImGui::TableHeadersRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TableHeader("#");
        for (auto i = 0; i < _tracks.size(); ++i) {
            ImGui::TableSetColumnIndex(i + 1);
            auto name = _tracks[i]->_name.c_str();
            ImGui::TableHeader(name);
        }

        ImGui::TableSetColumnIndex(0);
        ImGui::TableNextRow();
        for (auto i = 0; i < _tracks.size(); ++i) {
            ImGui::TableSetColumnIndex(i + 1);
            ImGui::PushID(i);
            _tracks[i]->render();
            ImGui::PopID();
        }

        ImGui::TableSetColumnIndex(static_cast<int>(_tracks.size() + 1));
        ImGui::PushID("MASTER RACK");
        _masterTrack->render();
        ImGui::PopID();

        ImGui::EndTable();
    }

    if (ImGui::Button("Add track")) {
        addTrack();
    }

    ImGui::Text(_statusMessage.c_str());

    {
        if (ImGui::IsKeyPressed(ImGuiKey_Space)) {
            if (_playing) {
                stop();
            } else {
                play();
            }
        }
    }

    ImGui::End();
}

void Composer::play() {
    _playing = true;
}

void Composer::stop() {
    _playing = false;
    _playPosition = PlayPosition{ ._line = 0, ._delay = 0 };
    _nextPlayPosition = PlayPosition{ ._line = 0, ._delay = 0 };
}

class AddTrackCommand : public Command {
public:
    AddTrackCommand(Track* track) : _track(track) {}
    void execute(Composer* composer) override {
        std::lock_guard<std::mutex> lock(composer->_audioEngine->mtx);
        composer->_tracks.push_back(std::move(_track));
    }
    void undo(Composer* composer) override {
        std::lock_guard<std::mutex> lock(composer->_audioEngine->mtx);
        _track = std::move(composer->_tracks.back());
        composer->_tracks.pop_back();
    }

    std::unique_ptr<Track> _track;

};

void Composer::addTrack() {
    std::stringstream name;
    name << "track " << this->_tracks.size() + 1;
    Track* track = new Track(name.str(), this);
    _commandManager.executeCommand(new AddTrackCommand(track));
}
