#include <cstdint>
#include <concepts>

namespace boe
{
    constexpr uint16_t START_OF_MESSAGE{0xBABA};

    namespace msg_type
    {
        // Member to Cboe
        constexpr uint8_t login_request         = 0x37;
        constexpr uint8_t logout_request        = 0x02;
        constexpr uint8_t client_heartbeat      = 0x03;
        constexpr uint8_t new_order             = 0x38;
        constexpr uint8_t cancel_order          = 0x39;

        // Cboe to Member
        constexpr uint8_t login_response        = 0x24;
        constexpr uint8_t logout                = 0x08;
        constexpr uint8_t server_heartbeat      = 0x09;
        constexpr uint8_t replay_complete       = 0x13;
        constexpr uint8_t order_acknowledgement = 0x25;
        constexpr uint8_t order_rejected        = 0x26;
        constexpr uint8_t order_cancelled       = 0x2A;
        constexpr uint8_t cancel_rejected       = 0x2B;
        constexpr uint8_t order_execution       = 0x2C;
    }

    namespace side
    {
        constexpr char buy  = '1';
        constexpr char sell = '2';
    }

    namespace ord_type
    {
        constexpr char market = '1';
        constexpr char limit  = '2';
    }

    namespace capacity
    {
        constexpr char customer         = 'C';
        constexpr char firm             = 'F';
        constexpr char market_maker     = 'M';
        constexpr char professional     = 'U';
    }

    namespace login_status 
    {
        constexpr char accepted         = 'A';
        constexpr char not_authorized   = 'N';
        constexpr char already_in_use   = 'B';
        constexpr char session_disabled = 'D';
        constexpr char invalid_session  = 'S';
        constexpr char invalid_structure = 'M';
    }

    namespace reject_reason 
    {
        constexpr char duplicate_cl_ord_id = 'D';
        constexpr char invalid_price       = 'K';
        constexpr char invalid_quantity    = 'Z';
        constexpr char unknown_symbol      = 'S';
        constexpr char administrative      = 'A';
        constexpr char not_logged_in       = '>';
    }

    namespace cancel_reject_reason
    {
        constexpr char unknown_order = 'I';
        constexpr char too_late      = 'J';
        constexpr char administrative = 'A';
    }

    namespace logout_reason
    {
        constexpr char user_requested      = 'U';
        constexpr char end_of_day          = 'E';
        constexpr char administrative      = 'A';
        constexpr char protocol_violation  = '!';
    }

    namespace cancel_reason 
    {
        constexpr char user_requested = 'U';
        constexpr char administrative = 'A';
    }

    namespace liquidity
    {
        constexpr char added   = 'A';
        constexpr char removed = 'R';
    }

    #pragma pack(push, 1)

    // Section 3.1.1 : First message sent on connect. Must be received before any
    // application messages. Gateway responds with login_response + replay_complete.
    // If credentials are invalid, gateway sends login_response with non-'A' status
    // and closes the connection immediately.
    struct login_request
    {
        uint16_t start_of_message{START_OF_MESSAGE};
        uint16_t message_length{}; 
        uint8_t  message_type{msg_type::login_request};
        uint8_t  matching_unit{};
        uint32_t sequence_number{};
        char     session_sub_id[4]{};
        char     username[4]{};
        char     password[10]{};
        uint8_t  number_of_param_groups{0x00};
    };

    // Section 3.1.2 : Sent by client to initiate graceful disconnect. Gateway
    // finishes sending any queued data, responds with logout, then closes the
    // TCP connection.
    struct logout_request
    {
        uint16_t start_of_message{START_OF_MESSAGE};
        uint16_t message_length{};
        uint8_t  message_type{msg_type::logout_request};
        uint8_t  matching_unit{};
        uint32_t sequence_number{};
    };

    // Section 3.2.2 : Sent by gateway to confirm disconnect, either in response
    // to a logout_request or unsolicited (protocol violation, end of day, idle
    // timeout). Client should not send any messages after receiving this.
    // LogoutReason: 'U' = user requested, 'E' = end of day,
    //               'A' = administrative, '!' = protocol violation
    struct logout
    {
    uint16_t start_of_message{START_OF_MESSAGE};
    uint16_t message_length{};
    uint8_t  message_type{msg_type::logout};
    uint8_t  matching_unit{0x00};
    uint32_t sequence_number{0x00};
    char     logout_reason{};
    char     logout_reason_text[60]{};
    uint32_t last_received_sequence_number{};
    uint8_t  number_of_units{0x00};
    };

    // Section 2.4 : Sent by either side when no data has been exchanged for 1
    // second. Resets the idle timer. If the gateway receives nothing for 5
    // seconds it sends logout and closes the connection.

    struct client_heartbeat
    {
        uint16_t start_of_message{START_OF_MESSAGE};
        uint16_t message_length{8};
        uint8_t  message_type{msg_type::client_heartbeat};
        uint8_t  matching_unit{};
        uint32_t sequence_number{};
    };

    // Section 2.4 : Gateway-side keepalive. Sent when no outbound data has been
    // sent for 1 second. Client should respond with client_heartbeat if it has
    // nothing else to send.
    struct server_heartbeat 
    {
        uint16_t start_of_message{START_OF_MESSAGE};
        uint16_t message_length{0x08};
        uint8_t  message_type{msg_type::server_heartbeat};
        uint8_t  matching_unit{0x00};
        uint32_t sequence_number{0x00};
    };

    // Section 3.2.4 : Sent immediately after login_response to signal that replay
    // is complete. Client must not send orders until this is received. This gateway
    // has no replay mechanism so replay_complete is always sent immediately with
    // no messages in between.
    struct replay_complete
    {
        uint16_t start_of_message{START_OF_MESSAGE};
        uint16_t message_length{0x08};
        uint8_t  message_type{msg_type::replay_complete};
        uint8_t  matching_unit{0x00};
        uint32_t sequence_number{0x00};
    };

    // Section 3.2.1 : Gateway response to login_request.
    // login_response_status: 'A' = accepted, 'N' = not authorized,
    // 'B' = session already in use, 'D' = session disabled.
    // Always followed by replay_complete on successful login.
    struct login_response
    {
        uint16_t start_of_message{START_OF_MESSAGE};
        uint16_t message_length{};
        uint8_t  message_type{msg_type::login_response};
        uint8_t  matching_unit{};
        uint32_t sequence_number{};
        char     login_response_status{};
        char     login_response_text[60]{};
        uint8_t  no_unspecified_unit_replay{};
        uint32_t last_received_sequence_number{};
        uint8_t  number_of_units{0x00};
        uint8_t  number_of_param_groups{0x00};
    };    

    // Section 4.1.1 : Submit a new limit or market order.
    // Side: '1' = Buy, '2' = Sell.
    // OrdType: '1' = Market, '2' = Limit (default if omitted).
    // Price is Binary Price — 4 implied decimal places ($1.50 = 15000).
    // ClOrdID must be unique among all currently live orders.
    // Gateway responds with order_acknowledgement or order_rejected.
    // If the order crosses the book, order_execution is also sent.
    struct new_order
    {
        uint16_t start_of_message{START_OF_MESSAGE};
        uint16_t message_length{};
        uint8_t  message_type{msg_type::new_order};
        uint8_t  matching_unit{};
        uint32_t sequence_number{};
        char     cl_ord_id[20]{};
        char     side{};
        uint32_t order_qty{};
        uint8_t  number_of_new_order_bitfields{2};
        uint8_t  bitfield1{0x14};  // Price + OrdType
        uint8_t  bitfield2{0x41};  // Symbol + Capacity
        int64_t  price{};          
        char     ord_type{};       // '1' = Market, '2' = Limit
        char     symbol[8]{};     
        char     capacity{'C'};
    };

    // Section 4.1.5 : Cancel a resting order by its original ClOrdID.
    // OrigClOrdID must match the ClOrdID of a live resting order.
    // Set OrigClOrdID to all zeroes for mass cancel.
    // Gateway responds with order_cancelled or cancel_rejected.
    // No ClOrdID field — a cancel creates nothing new, it only references
    // an existing order.
    struct cancel_order
    {
        uint16_t start_of_message{START_OF_MESSAGE};
        uint16_t message_length{};
        uint8_t  message_type{msg_type::cancel_order};
        uint8_t  matching_unit{};
        uint32_t sequence_number{};
        char     orig_cl_ord_id[20]{};
        uint8_t  number_of_cancel_order_bitfields{0};
    };

    // Section 4.2.1 : Sent when a new order is accepted and resting on the book.
    // Sequenced. OrderID is gateway-assigned, independent of ClOrdID.
    // If the order immediately crosses and fills, order_execution follows.
    struct order_acknowledgement
    {
        uint16_t start_of_message{START_OF_MESSAGE};
        uint16_t message_length{};
        uint8_t  message_type{msg_type::order_acknowledgement};
        uint8_t  matching_unit{0x00};
        uint32_t sequence_number{};
        uint64_t transaction_time{};    
        char     cl_ord_id[20]{};       
        uint64_t order_id{};            
        uint8_t  reserved_internal{0x00};
        uint8_t  num_return_bitfields{0};
    };

    // Section 4.2.4 : Sent when a new order fails validation before reaching
    // the book. Unsequenced — rejections are never replayed.
    // order_reject_reason codes: 'D' = duplicate ClOrdID, 'K' = invalid price,
    // 'Z' = invalid quantity, 'S' = unknown symbol, 'A' = administrative.
    struct order_rejected
    {
        uint16_t start_of_message{START_OF_MESSAGE};
        uint16_t message_length{};
        uint8_t  message_type{msg_type::order_rejected};
        uint8_t  matching_unit{0x00};
        uint32_t sequence_number{0x00};
        uint64_t transaction_time{};
        char     cl_ord_id[20]{};
        char     order_reject_reason{};
        char     text[60]{};
        uint8_t  reserved_internal{0x00};
        uint8_t  num_return_bitfields{0};
    };
    
    // Section 4.2.11 : Sent when a cancel succeeds and the order is removed
    // from the book. Sequenced.
    // cancel_reason: 'U' = user requested (default), 'A' = administrative.
    struct order_cancelled
    {
        uint16_t start_of_message{START_OF_MESSAGE};
        uint16_t message_length{};
        uint8_t  message_type{msg_type::order_cancelled};
        uint8_t  matching_unit{0x00};
        uint32_t sequence_number{};
        uint64_t transaction_time{};
        char     cl_ord_id[20]{};
        char     cancel_reason{'U'};
        uint8_t  reserved_internal{0x00};
        uint8_t  num_return_bitfields{0};
    };

    // Section 4.2.14 : Sent when a cancel cannot be processed. Unsequenced.
    // cancel_reject_reason: 'I' = unknown order (ClOrdID not found),
    // 'J' = too late (order already filled or cancelled).
    struct cancel_rejected
    {
        uint16_t start_of_message{START_OF_MESSAGE};
        uint16_t message_length{};
        uint8_t  message_type{msg_type::cancel_rejected};
        uint8_t  matching_unit{0x00};
        uint32_t sequence_number{0x00};
        uint64_t transaction_time{};
        char     cl_ord_id[20]{};
        char     cancel_reject_reason{};
        char     text[60]{};
        uint8_t  reserved_internal{0x00};
        uint8_t  num_return_bitfields{0};
    };

    // Section 4.2.15 : Sent when a fill occurs. Sequenced.
    // Sent TWICE per fill — once to the aggressor (BaseLiquidityIndicator = 'R',
    // Removed Liquidity) and once to the resting order owner
    // (BaseLiquidityIndicator = 'A', Added Liquidity). Both messages carry the
    // same ExecID and LastPx.
    // LeavesQty = 0 means fully filled. LeavesQty > 0 means partial fill,
    // remainder still resting on book.
    struct order_execution {
        uint16_t start_of_message{START_OF_MESSAGE};
        uint16_t message_length{};
        uint8_t  message_type{msg_type::order_execution};
        uint8_t  matching_unit{0x00};
        uint32_t sequence_number{};
        uint64_t transaction_time{};
        char     cl_ord_id[20]{};
        uint64_t exec_id{};
        uint32_t last_shares{};
        int64_t  last_px{};           
        uint32_t leaves_qty{};
        char     base_liquidity_indicator{};
        char     sub_liquidity_indicator{0x00};
        char     contra_broker[4]{};
        uint8_t  reserved_internal{0x00};
        uint8_t  num_return_bitfields{0};
    };
    #pragma pack(pop)

    template<typename T>
    concept BoeMessage = requires(T msg) {
        { msg.message_length } -> std::convertible_to<uint16_t>;
        { msg.message_type   } -> std::convertible_to<uint8_t>;
    };

    template<BoeMessage T>
    T make_message() {
        T msg{};
        msg.message_length = static_cast<uint16_t>(sizeof(T) - 2);
        return msg;
    }
}
