#pragma once

#include <cstdint>
#include <cstring>
#include "boe_protocol.hpp"

// starts the recv loop for a connected client
void run_session(int fd);

// reads exactly n_bytes from fd into buf
// returns false on disconnect or error
bool recv_exact(int fd, void* buf, int n_bytes);

// sends a BOE message to the client
void send_message(int fd, const void* data, int len);