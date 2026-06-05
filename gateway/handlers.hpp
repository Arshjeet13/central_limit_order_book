#pragma once

#include "session.hpp"
#include "boe/protocol.hpp"
#include <cstdint>
#include <utility>
#include <variant>

std::pair<boe::login_response, boe::replay_complete> handle_login_request(Session& session,
                                                                          const boe::login_request);

boe::logout handle_logout_request(Session& session,
                                  const boe::logout_request);

std::variant<boe::order_acknowledgement, boe::order_rejected> handle_new_order(Session& session,
                                                                               const boe::new_order);

std::variant<boe::order_cancelled, boe::cancel_rejected> handle_cancel_order(Session& session,
                                                                             const boe::cancel_order);

// boe::server_heartbeat send_server_heartbeat();
// will be handled when basic functionality is achieved
