#pragma once
#include <map>
#include <string>

short noteToNumber(std::string note_name);
std::string numberToNote(int16_t key);

constexpr short NOTE_OFF = 128;
constexpr short NOTE_NONE = -1;

namespace Midi {
constexpr char OFF[] = "OFF";
constexpr char C0[] = "C0"; constexpr char C0_SHARP[] = "C0#"; constexpr char D0_FLAT[] = "D0b";
constexpr char D0[] = "D0"; constexpr char D0_SHARP[] = "D0#"; constexpr char E0_FLAT[] = "E0b";
constexpr char E0[] = "E0";
constexpr char F0[] = "F0"; constexpr char F0_SHARP[] = "F0#"; constexpr char G0_FLAT[] = "G0b";
constexpr char G0[] = "G0"; constexpr char G0_SHARP[] = "G0#"; constexpr char A0_FLAT[] = "A0b";
constexpr char A0[] = "A0"; constexpr char A0_SHARP[] = "A0#"; constexpr char B0_FLAT[] = "B0b";
constexpr char B0[] = "B0";
constexpr char C1[] = "C1"; constexpr char C1_SHARP[] = "C1#"; constexpr char D1_FLAT[] = "D1b";
constexpr char D1[] = "D1"; constexpr char D1_SHARP[] = "D1#"; constexpr char E1_FLAT[] = "E1b";
constexpr char E1[] = "E1";
constexpr char F1[] = "F1"; constexpr char F1_SHARP[] = "F1#"; constexpr char G1_FLAT[] = "G1b";
constexpr char G1[] = "G1"; constexpr char G1_SHARP[] = "G1#"; constexpr char A1_FLAT[] = "A1b";
constexpr char A1[] = "A1"; constexpr char A1_SHARP[] = "A1#"; constexpr char B1_FLAT[] = "B1b";
constexpr char B1[] = "B1";
constexpr char C2[] = "C2"; constexpr char C2_SHARP[] = "C2#"; constexpr char D2_FLAT[] = "D2b";
constexpr char D2[] = "D2"; constexpr char D2_SHARP[] = "D2#"; constexpr char E2_FLAT[] = "E2b";
constexpr char E2[] = "E2";
constexpr char F2[] = "F2"; constexpr char F2_SHARP[] = "F2#"; constexpr char G2_FLAT[] = "G2b";
constexpr char G2[] = "G2"; constexpr char G2_SHARP[] = "G2#"; constexpr char A2_FLAT[] = "A2b";
constexpr char A2[] = "A2"; constexpr char A2_SHARP[] = "A2#"; constexpr char B2_FLAT[] = "B2b";
constexpr char B2[] = "B2";
constexpr char C3[] = "C3"; constexpr char C3_SHARP[] = "C3#"; constexpr char D3_FLAT[] = "D3b";
constexpr char D3[] = "D3"; constexpr char D3_SHARP[] = "D3#"; constexpr char E3_FLAT[] = "E3b";
constexpr char E3[] = "E3";
constexpr char F3[] = "F3"; constexpr char F3_SHARP[] = "F3#"; constexpr char G3_FLAT[] = "G3b";
constexpr char G3[] = "G3"; constexpr char G3_SHARP[] = "G3#"; constexpr char A3_FLAT[] = "A3b";
constexpr char A3[] = "A3"; constexpr char A3_SHARP[] = "A3#"; constexpr char B3_FLAT[] = "B3b";
constexpr char B3[] = "B3";
constexpr char C4[] = "C4"; constexpr char C4_SHARP[] = "C4#"; constexpr char D4_FLAT[] = "D4b";
constexpr char D4[] = "D4"; constexpr char D4_SHARP[] = "D4#"; constexpr char E4_FLAT[] = "E4b";
constexpr char E4[] = "E4";
constexpr char F4[] = "F4"; constexpr char F4_SHARP[] = "F4#"; constexpr char G4_FLAT[] = "G4b";
constexpr char G4[] = "G4"; constexpr char G4_SHARP[] = "G4#"; constexpr char A4_FLAT[] = "A4b";
constexpr char A4[] = "A4"; constexpr char A4_SHARP[] = "A4#"; constexpr char B4_FLAT[] = "B4b";
constexpr char B4[] = "B4";
constexpr char C5[] = "C5"; constexpr char C5_SHARP[] = "C5#"; constexpr char D5_FLAT[] = "D5b";
constexpr char D5[] = "D5"; constexpr char D5_SHARP[] = "D5#"; constexpr char E5_FLAT[] = "E5b";
constexpr char E5[] = "E5";
constexpr char F5[] = "F5"; constexpr char F5_SHARP[] = "F5#"; constexpr char G5_FLAT[] = "G5b";
constexpr char G5[] = "G5"; constexpr char G5_SHARP[] = "G5#"; constexpr char A5_FLAT[] = "A5b";
constexpr char A5[] = "A5"; constexpr char A5_SHARP[] = "A5#"; constexpr char B5_FLAT[] = "B5b";
constexpr char B5[] = "B5";
constexpr char C6[] = "C6"; constexpr char C6_SHARP[] = "C6#"; constexpr char D6_FLAT[] = "D6b";
constexpr char D6[] = "D6"; constexpr char D6_SHARP[] = "D6#"; constexpr char E6_FLAT[] = "E6b";
constexpr char E6[] = "E6";
constexpr char F6[] = "F6"; constexpr char F6_SHARP[] = "F6#"; constexpr char G6_FLAT[] = "G6b";
constexpr char G6[] = "G6"; constexpr char G6_SHARP[] = "G6#"; constexpr char A6_FLAT[] = "A6b";
constexpr char A6[] = "A6"; constexpr char A6_SHARP[] = "A6#"; constexpr char B6_FLAT[] = "B6b";
constexpr char B6[] = "B6";
constexpr char C7[] = "C7"; constexpr char C7_SHARP[] = "C7#"; constexpr char D7_FLAT[] = "D7b";
constexpr char D7[] = "D7"; constexpr char D7_SHARP[] = "D7#"; constexpr char E7_FLAT[] = "E7b";
constexpr char E7[] = "E7";
constexpr char F7[] = "F7"; constexpr char F7_SHARP[] = "F7#"; constexpr char G7_FLAT[] = "G7b";
constexpr char G7[] = "G7"; constexpr char G7_SHARP[] = "G7#"; constexpr char A7_FLAT[] = "A7b";
constexpr char A7[] = "A7"; constexpr char A7_SHARP[] = "A7#"; constexpr char B7_FLAT[] = "B7b";
constexpr char B7[] = "B7";
constexpr char C8[] = "C8"; constexpr char C8_SHARP[] = "C8#"; constexpr char D8_FLAT[] = "D8b";
constexpr char D8[] = "D8"; constexpr char D8_SHARP[] = "D8#"; constexpr char E8_FLAT[] = "E8b";
constexpr char E8[] = "E8";
constexpr char F8[] = "F8"; constexpr char F8_SHARP[] = "F8#"; constexpr char G8_FLAT[] = "G8b";
constexpr char G8[] = "G8"; constexpr char G8_SHARP[] = "G8#"; constexpr char A8_FLAT[] = "A8b";
constexpr char A8[] = "A8"; constexpr char A8_SHARP[] = "A8#"; constexpr char B8_FLAT[] = "B8b";
constexpr char B8[] = "B8";
constexpr char C9[] = "C9";
}

extern std::map<std::string, int> gMidiSymToNum;
extern std::map<int, std::string> gMidiNumToSym;
