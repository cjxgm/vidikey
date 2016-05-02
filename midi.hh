#pragma once
#include <alsa/asoundlib.h>

struct midi
{
    midi();
    ~midi();

    void note_on(char note, char velocity);
    void note_off(char note, char velocity);

private:
    snd_seq_t* seq;
    int port;

    snd_seq_event_t make_event();
    void play_event(snd_seq_event_t & ev);
};

