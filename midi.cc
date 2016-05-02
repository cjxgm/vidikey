#include "midi.hh"
#include <iostream>

static void alsa_assert(int err)
{
    if (err >= 0) return;
    std::cerr << "alsa failure " << err << "\n";
    std::exit(-err);
}

midi::midi()
{
    alsa_assert(snd_seq_open(&seq, "default", SND_SEQ_OPEN_OUTPUT, {}));
    snd_seq_set_client_name(seq, "Vidikey Master");
    port = snd_seq_create_simple_port(seq, "out",
            SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ,
            SND_SEQ_PORT_TYPE_MIDI_GENERIC);
}

midi::~midi()
{
    snd_seq_delete_simple_port(seq, port);
    snd_seq_close(seq);
}

void midi::note_on(char note, char velocity)
{
    auto ev = make_event();
    snd_seq_ev_set_noteon(&ev, 0, note, velocity);
    play_event(ev);
}

void midi::note_off(char note, char velocity)
{
    auto ev = make_event();
    snd_seq_ev_set_noteoff(&ev, 0, note, velocity);
    play_event(ev);
}

snd_seq_event_t midi::make_event()
{
    snd_seq_event_t ev;
    snd_seq_ev_clear(&ev);
    snd_seq_ev_set_source(&ev, port);
    snd_seq_ev_set_subs(&ev);
    snd_seq_ev_set_direct(&ev);
    return ev;
}

void midi::play_event(snd_seq_event_t & ev)
{
    snd_seq_event_output_direct(seq, &ev);
}

