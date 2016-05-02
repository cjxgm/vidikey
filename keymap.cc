#include "keymap.hh"
#include <map>
#include <linux/input-event-codes.h>

static std::map<int, int> key_to_note {
    {KEY_Z,  0}, {KEY_S,  1},
    {KEY_X,  2}, {KEY_D,  3},
    {KEY_C,  4},
    {KEY_V,  5}, {KEY_G,  6},
    {KEY_B,  7}, {KEY_H,  8},
    {KEY_N,  9}, {KEY_J, 10},
    {KEY_M, 11},
    {KEY_Q, 12}, {KEY_2, 13},
    {KEY_W, 14}, {KEY_3, 15},
    {KEY_E, 16},
    {KEY_R, 17}, {KEY_5, 18},
    {KEY_T, 19}, {KEY_6, 20},
    {KEY_Y, 21}, {KEY_7, 22},
    {KEY_U, 23},
    {KEY_I, 24}, {KEY_9, 25},
    {KEY_O, 26}, {KEY_0, 27},
    {KEY_P, 28},
};

bool keymap::is_quit(int code)
{
    return (code == KEY_ESC);
}

int keymap::midi_note(int code)
{
    auto note = key_to_note.find(code);
    if (note == key_to_note.end()) return -1;
    return note->second;
}

bool keymap::is_octave_up(int code)
{
    return (code == KEY_RIGHTBRACE);
}

bool keymap::is_octave_down(int code)
{
    return (code == KEY_LEFTBRACE);
}

