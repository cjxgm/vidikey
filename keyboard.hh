#pragma once
#include <uv.h>

struct keyboard_t
{
    void* data;

    std::string device_path;
};

using key_cb = void (keyboard_t* kbd, bool isdown, int keycode);

void keyboard_init(uv_loop_t* loop, keyboard_t* kbd);
void keyboard_close(keyboard_t* kbd);
void keyboard_read_start(keyboard_t* kbd, key_cb* on_key);

