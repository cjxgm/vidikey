#pragma once

struct keymap
{
    bool is_quit(int code);
    int midi_note(int code);
};

