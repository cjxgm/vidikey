#pragma once

struct keymap
{
    bool is_quit(int code) { return false; }
    int midi_note(int code) { return -1; }
};

