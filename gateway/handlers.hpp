#pragma once

#include "session.hpp"
#include "boe/protocol.hpp"
#include <cstdint>
#include <utility>
#include <variant>

void handle_login_request(Session& session, const boe::login_request login_request);

void handle_logout_request(Session& session, const boe::logout_request);

void handle_new_order(Session& session, const boe::new_order);

void handle_cancel_order(Session& session, const boe::cancel_order);

boe::login_response make_login_response(bool valid);

boe::replay_complete make_replay_complete();

void send_message(uint8_t* buffer);
// boe::server_heartbeat send_server_heartbeat();
// will be handled when basic functionality is achieved
