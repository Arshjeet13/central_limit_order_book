#pragma once

#include <cstdint>
#include "session.hpp"

bool parse_message(Session& session, const uint8_t* buffer);