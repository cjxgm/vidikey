#include "uv.hh"
#include <iostream>
#include <deque>
#include "midi.hh"

static void alloc_buf(uv_handle_t*, size_t size, uv_buf_t* buf)
{
    *buf = uv_buf_init(new char[size], size);
}

struct server
{
    server()
    {
        service.data = this;
        uv_assert(uv_tcp_init(uv_default_loop(), &service));

        ::sockaddr_in addr;
        uv_assert(uv_ip4_addr("0.0.0.0", 2277, &addr));
        uv_assert(uv_tcp_bind(&service, (::sockaddr*)&addr, 0));

        uv_assert(uv_tcp_nodelay(&service, true));
        uv_assert(uv_listen((uv_stream_t*) &service, 32, &connection));
    }

private:
    uv_tcp_t service;
    midi m;

    struct connection_info
    {
        server* server;
        std::deque<char> buffer;
        std::string peername;
    };

    static void connection(uv_stream_t* s, int status)
    {
        uv_assert(status);

        auto conn = new uv_tcp_t;
        uv_assert(uv_tcp_init(uv_default_loop(), conn));
        uv_assert(uv_accept(s, (uv_stream_t*) conn));

        ::sockaddr_in addr;
        int unused;
        char name[32];
        uv_assert(uv_tcp_getpeername((uv_tcp_t*) conn, (::sockaddr*) &addr, &unused));
        uv_assert(uv_ip4_name(&addr, name, sizeof(name)));

        auto info = new connection_info{
            .server = (server*) s->data,
            .peername = name,
        };
        conn->data = info;

        std::cerr << "> accept " << info->peername << "\n";
        uv_assert(uv_read_start((uv_stream_t*) conn, &alloc_buf, &serve));
    }

    static void serve(uv_stream_t* conn, ssize_t nread, uv_buf_t const* buf)
    {
        auto info = (connection_info*) conn->data;
        auto& b = info->buffer;

        if (nread == UV_EOF) {
            std::cerr << "[" << info->peername << "] closed\n";
            if (buf->base) delete[] buf->base;
            delete info;
            uv_close((uv_handle_t*) conn, nullptr);
            return;
        }
        uv_assert(nread);

        for (int i=0; i<nread; i++)
            b.emplace_back(buf->base[i]);
        delete[] buf->base;

        while (b.size() >= 3) {
            char type = b[0];
            char note = b[1];
            char velocity = b[2];
            b.erase(begin(b), begin(b)+3);

            std::cerr << "[" << info->peername << "] " << (int)type << " " << (int)note << " " << (int)velocity << "\n";
            auto& m = info->server->m;
            switch (type) {
                case 0: break;
                case 1: m.note_on(note, velocity); break;
                case 2: m.note_off(note, velocity); break;
                default:
                    std::cerr << "!!!! invalid !!!!\n";
                    break;
            }
        }
    }
};

int main()
{
    std::cerr << "Vidikey Master (server)\n";
    server s;
    std::cerr << "- listening on 0.0.0.0:2277\n";
    uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}

