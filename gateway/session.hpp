#pragma once

#include <cstdint>
#include <cstring>

struct Session
{
    int      fd{-1};
    bool     logged_in{false};
    uint32_t next_seq_num{1};
    char     username[4]{};
};

// starts the recv loop for a connected client
void run_session(int fd);

// reads exactly n_bytes from fd into buf
// returns false on disconnect or error
bool recv_exact(int fd, uint8_t* buffer, int n_bytes);

// sends a BOE message to the client
void send_message(int fd, const void* data, int len);