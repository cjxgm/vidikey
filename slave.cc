// ml:run = $bin 127.0.0.1
#include "uv.hh"
#include <iostream>
#include <deque>
#include "keymap.hh"
#include "keyboard.hh"

struct client
{
    client(std::string const& server_addr, int server_port)
    {
        keyboard_init(uv_default_loop(), &kbd);

        service.data = this;
        uv_assert(uv_tcp_init(uv_default_loop(), &service));
        uv_assert(uv_tcp_nodelay(&service, true));

        ::sockaddr_in addr;
        uv_assert(uv_ip4_addr(server_addr.c_str(), server_port, &addr));

        auto req = new uv_connect_t;
        uv_assert(uv_tcp_connect(req, &service, (::sockaddr*)&addr, &connection));
    }

private:
    uv_tcp_t service;
    keyboard_t kbd;
    keymap km;
    int octave = 3;
    int velocity = 9;

    static void connection(uv_connect_t* req, int status)
    {
        uv_assert(status);
        std::cerr << "> connected\n";
        auto conn = req->handle;
        delete req;

        auto c = (client*) conn->data;
        auto& kbd = c->kbd;
        kbd.data = c;
        keyboard_read_start(&kbd, &input);
    }

    static void input(keyboard_t* kbd, bool isdown, int key)
    {
        auto c = (client*) kbd->data;
        auto& km = c->km;
        auto& conn = c->service;

        std::cerr << "\e[G\e[K";
        if (km.is_quit(key)) {
            uv_close((uv_handle_t*) &conn, nullptr);
            keyboard_close(kbd);
            return;
        }

        if (km.is_octave_up(key)) {
            if (isdown && c->octave < 7)
                c->octave++;
            std::cerr << "octave " << c->octave << "\n";
            return;
        }

        if (km.is_octave_down(key)) {
            if (isdown && c->octave > 0)
                c->octave--;
            std::cerr << "octave " << c->octave << "\n";
            return;
        }

        if (km.is_velocity_up(key)) {
            if (isdown && c->velocity < 12)
                c->velocity++;
            std::cerr << "velocity " << c->velocity << "\n";
            return;
        }

        if (km.is_velocity_down(key)) {
            if (isdown && c->velocity > 1)
                c->velocity--;
            std::cerr << "velocity " << c->velocity << "\n";
            return;
        }

        auto note = km.midi_note(key);
        if (note == -1) {
            std::cerr << (isdown ? "* " : "  ") << key << "\n";
            return;
        }
        note += 12 * c->octave;
        std::cerr << (isdown ? "# " : ". ") << note << "\n";

        auto buf_data = new char[3]{
            (char)(isdown ? 1 : 2),
            (char)note,
            (char)(c->velocity * 10),
        };
        auto buf = uv_buf_init(buf_data, 3);
        auto req = new uv_write_t;
        req->data = buf_data;
        uv_write(req, (uv_stream_t*) &conn, &buf, 1, &sent);
    }

    static void sent(uv_write_t* req, int status)
    {
        uv_assert(status);
        auto buf_data = (char*) req->data;
        delete[] buf_data;
        delete req;
    }
};

int main(int argc, char* argv[])
{
    std::string server_addr;
    if (argc > 1) server_addr = argv[1];
    else {
        std::cerr << "usage: slave SERVER_ADDR [SERVER_PORT]\n";
        return 1;
    }

    int server_port = 2277;
    if (argc > 2) server_addr = std::stoi(argv[2]);

    std::cerr << "Vidikey Slave (client)\n";
    client c{server_addr, server_port};
    std::cerr << "- connect to " << server_addr << ":" << server_port << "\n";
    uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}

