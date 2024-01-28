#include "Midi.h"
#include <map>
#include <string>


std::map<std::string, int> table = {
    {"OFF", NOTE_OFF},
    {"C0", 12}, {"C0#", 13}, {"D0b", 13}, {"D0", 14}, {"D0#", 15}, {"E0b", 15}, {"E0", 16},
    {"F0", 17}, {"F0#", 18}, {"G0b", 18}, {"G0", 19}, {"G0#", 20}, {"A0b", 20}, {"A0", 21},
    {"A0#", 22}, {"B0b", 22}, {"B0", 23},
    {"C1", 24}, {"C1#", 25}, {"D1b", 25}, {"D1", 26}, {"D1#", 27}, {"E1b", 27}, {"E1", 28},
    {"F1", 29}, {"F1#", 30}, {"G1b", 30}, {"G1", 31}, {"G1#", 32}, {"A1b", 32}, {"A1", 33},
    {"A1#", 34}, {"B1b", 34}, {"B1", 35},
    {"C2", 36}, {"C2#", 37}, {"D2b", 37}, {"D2", 38}, {"D2#", 39}, {"E2b", 39}, {"E2", 40},
    {"F2", 41}, {"F2#", 42}, {"G2b", 42}, {"G2", 43}, {"G2#", 44}, {"A2b", 44}, {"A2", 45},
    {"A2#", 46}, {"B2b", 46}, {"B2", 47},
    {"C3", 48}, {"C3#", 49}, {"D3b", 49}, {"D3", 50}, {"D3#", 51}, {"E3b", 51}, {"E3", 52},
    {"F3", 53}, {"F3#", 54}, {"G3b", 54}, {"G3", 55}, {"G3#", 56}, {"A3b", 56}, {"A3", 57},
    {"A3#", 58}, {"B3b", 58}, {"B3", 59},
    {"C4", 60}, {"C4#", 61}, {"D4b", 61}, {"D4", 62}, {"D4#", 63}, {"E4b", 63}, {"E4", 64},
    {"F4", 65}, {"F4#", 66}, {"G4b", 66}, {"G4", 67}, {"G4#", 68}, {"A4b", 68}, {"A4", 69},
    {"A4#", 70}, {"B4b", 70}, {"B4", 71},
    {"C5", 72}, {"C5#", 73}, {"D5b", 73}, {"D5", 74}, {"D5#", 75}, {"E5b", 75}, {"E5", 76},
    {"F5", 77}, {"F5#", 78}, {"G5b", 78}, {"G5", 79}, {"G5#", 80}, {"A5b", 80}, {"A5", 81},
    {"A5#", 82}, {"B5b", 82}, {"B5", 83},
    {"C6", 84}, {"C6#", 85}, {"D6b", 85}, {"D6", 86}, {"D6#", 87}, {"E6b", 87}, {"E6", 88},
    {"F6", 89}, {"F6#", 90}, {"G6b", 90}, {"G6", 91}, {"G6#", 92}, {"A6b", 92}, {"A6", 93},
    {"A6#", 94}, {"B6b", 94}, {"B6", 95},
    {"C7", 96}, {"C7#", 97}, {"D7b", 97}, {"D7", 98}, {"D7#", 99}, {"E7b", 99}, {"E7", 100},
    {"F7", 101}, {"F7#", 102}, {"G7b", 102}, {"G7", 103}, {"G7#", 104}, {"A7b", 104}, {"A7", 105},
    {"A7#", 106}, {"B7b", 106}, {"B7", 107},
    {"C8", 108}, {"C8#", 109}, {"D8b", 109}, {"D8", 110}, {"D8#", 111}, {"E8b", 111}, {"E8", 112},
    {"F8", 113}, {"F8#", 114}, {"G8b", 114}, {"G8", 115}, {"G8#", 116}, {"A8b", 116}, {"A8", 117},
    {"A8#", 118}, {"B8b", 118}, {"B8", 119},
    {"C9", 120}
};

short noteToNumber(std::string note_name) {
    auto x = table.find(note_name);
    if (x == table.end()) {
        return NOTE_NONE;
    }
    return static_cast<int16_t>(x->second);
}

std::map<int, std::string> createReverseMap() {
    std::map<int, std::string> reverseTable;
    for (const auto& pair : table) {
        if (reverseTable.find(pair.second) == reverseTable.end()) { // まだ追加されていない場合
            reverseTable[pair.second] = pair.first;
        }
    }
    return reverseTable;
}

std::string numberToNote(int16_t key) {
    static const std::map<int, std::string> reverseTable = createReverseMap();
    auto it = reverseTable.find(key);
    if (it != reverseTable.end()) {
        return it->second;
    }
    return "";
}
