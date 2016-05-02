#include "uv.hh"
#include <iostream>

void uv_assert(int err)
{
    if (err >= 0) return;
    std::cerr << "* error: [" << uv_err_name(err) << "] " << uv_strerror(err) << "\n";
    std::exit(-err);
}

