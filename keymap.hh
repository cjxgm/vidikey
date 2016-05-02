#pragma once

struct keymap
{
    bool is_quit(int code);
    int midi_note(int code);
    bool is_octave_up(int code);
    bool is_octave_down(int code);
    bool is_velocity_up(int code);
    bool is_velocity_down(int code);
};

