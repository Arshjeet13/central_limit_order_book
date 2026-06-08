#pragma once

#include "session.hpp"
#include "../engine/engine.hpp"
#include <cstdint>

bool parse_message(Session& session, const uint8_t* buffer, MatchingEngine& engine);