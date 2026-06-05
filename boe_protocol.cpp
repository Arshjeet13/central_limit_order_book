#include <cstdint>

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

    #pragma pack(push, 1)
    struct login_request
    {
        uint16_t start_of_message{START_OF_MESSAGE};
        uint16_t message_length{27}; 
        uint8_t  message_type{msg_type::login_request};
        uint8_t  matching_unit{};
        uint32_t sequence_number{};
        char     session_sub_id[4]{};
        char     username[4]{};
        char     password[10]{};
        uint8_t  number_of_param_groups{0x00};
    };

    struct logout_request
    {
        uint16_t start_of_message{START_OF_MESSAGE};
        uint16_t message_length{8};
        uint8_t  message_type{msg_type::logout_request};
        uint8_t  matching_unit{};
        uint32_t sequence_number{};
    };

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

    // From section 2.4 : if no data has been sent in either direction for 1 second, 
    // both sides send heartbeats. If the server receives nothing from you for 5 seconds
    // it sends a Logout and closes the connection.
    struct client_heartbeat
    {
        uint16_t start_of_message{START_OF_MESSAGE};
        uint16_t message_length{8};
        uint8_t  message_type{msg_type::client_heartbeat};
        uint8_t  matching_unit{};
        uint32_t sequence_number{};
    };

    struct server_heartbeat 
    {
        uint16_t start_of_message{START_OF_MESSAGE};
        uint16_t message_length{0x08};
        uint8_t  message_type{msg_type::server_heartbeat};
        uint8_t  matching_unit{0x00};
        uint32_t sequence_number{0x00};
    };

    struct replay_complete
    {
        uint16_t start_of_message{START_OF_MESSAGE};
        uint16_t message_length{0x08};
        uint8_t  message_type{msg_type::replay_complete};
        uint8_t  matching_unit{0x00};
        uint32_t sequence_number{0x00};
    };

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
    
    struct order_cancelled {
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
}
