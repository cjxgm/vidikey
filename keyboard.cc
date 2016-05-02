#include "keyboard.hh"
#include <iostream>
#include <experimental/optional>
using std::experimental::optional;

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/input.h>

static bool is_keyboard(std::string const& device_path)
{
    int fd = ::open(device_path.c_str(), O_RDONLY);
    if (fd < 0) return false;

    unsigned long bits;
    if (ioctl(fd, EVIOCGBIT(0, sizeof(bits)), &bits) < 0) {
        ::close(fd);
        return false;
    }

    bool result = (bits & (1 << EV_KEY)) && (bits & (1 << EV_REP));
    ::close(fd);
    return result;
}

static optional<std::string> find_keyboard()
{
    for (int i=0;; i++) {
        std::string path = "/dev/input/event" + std::to_string(i);
        if (is_keyboard(path))
            return path;
        if (errno == ENOENT) break;     // "no such file" means the end of searching
    }
    return {};
}

void keyboard_init(uv_loop_t* loop, keyboard_t* kbd)
{
    auto opt_kbd = find_keyboard();
    if (!opt_kbd) {
        std::cerr << "keyboard not found.\n";
        std::exit(1);
    }
    kbd->device_path = std::move(*opt_kbd);
    kbd->loop = loop;
}

void keyboard_close(keyboard_t* kbd)
{
    std::cerr << "TODO: keyboard close\n";
    std::exit(0);
}

struct keyboard_info
{
    keyboard_t* kbd;
    key_cb* on_key;
    ::input_event ev;
    uv_file fd;
};

static void read_next_reuse(uv_fs_t* req);
static void read_finished(uv_fs_t* req)
{
    auto nread = req->result;
    uv_assert(nread);

    auto info = (keyboard_info*) req->data;
    auto& ev = info->ev;
    if (ev.type == EV_KEY && (ev.value == 1 || ev.value == 0))
        info->on_key(info->kbd, ev.value == 1, ev.code);

    read_next_reuse(req);
}

static void read_next(uv_fs_t* req)
{
    auto info = (keyboard_info*) req->data;
    auto buf = uv_buf_init((char*)&info->ev, sizeof(info->ev));
    uv_fs_read(req->loop, req, info->fd, &buf, 1, -1, &read_finished);
}

static void read_next_reuse(uv_fs_t* req)
{
    uv_fs_req_cleanup(req);
    read_next(req);
}

static void opened(uv_fs_t* req)
{
    uv_assert(req->result);

    auto info = (keyboard_info*) req->data;
    info->fd = req->result;

    uv_fs_req_cleanup(req);
    read_next(req);
}

void keyboard_read_start(keyboard_t* kbd, key_cb* on_key)
{
    auto req = new uv_fs_t;
    kbd->req = req;
    req->data = new keyboard_info{
        .kbd = kbd,
        .on_key = on_key,
    };
    uv_assert(uv_fs_open(kbd->loop, req, kbd->device_path.c_str(), O_RDONLY, 0, opened));
}

