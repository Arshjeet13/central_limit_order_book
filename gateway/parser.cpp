#include "parser.hpp"
#include "session.hpp"
#include "handlers.hpp"
#include "boe/protocol.hpp"

#include <cstdint>   
#include <cstring>    

bool parse_message(Session& session, const uint8_t* buffer){
    uint8_t message_type{};
    message_type = buffer[4]; // since message_type is stored in the 4th byte in every message

    switch(message_type){
        case boe::msg_type::login_request:     
            handle_login_request(session, *reinterpret_cast<const boe::login_request*> (buffer));   
            break;

        case boe::msg_type::logout_request:
            handle_logout_request(session, *reinterpret_cast<const boe::logout_request*> (buffer));
            break;

        case boe::msg_type::new_order:
            handle_new_order(session, *reinterpret_cast<const boe::new_order*> (buffer));
            break;

        case boe::msg_type::cancel_order:
            handle_cancel_order(session, *reinterpret_cast<const boe::cancel_order*> (buffer));
            break;

        // case boe::msg_type::client_heartbeat:
        // will be handled when basic functionality is achieved

        default:
            // TODO : add better functionality
            return false;
    }

    return true;
}