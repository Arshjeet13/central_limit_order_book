#include <cstdint>

namespace boe
{
    #pragma pack(push, 1)

    struct login_request
    {
        uint16_t start_of_message{0xBABA};
        uint16_t message_length{27}; 
        uint8_t  message_type{};
        uint8_t  matching_unit{};
        uint32_t sequence_number{};
        char     session_sub_id[4]{};
        char     username[4]{};
        char     password[10]{};
        uint8_t  number_of_param_groups{0x00};
    };

    struct logout_request
    {
        uint16_t start_of_message{0xBABA};
        uint16_t message_length{8};
        uint8_t  message_type{};
        uint8_t  matching_unit{};
        uint32_t sequence_number{};
    };

    // From section 2.4 : if no data has been sent in either direction for 1 second, 
    // both sides send heartbeats. If the server receives nothing from you for 5 seconds
    // it sends a Logout and closes the connection.
    struct client_hearbeat
    {
        uint16_t start_of_message{0xBABA};
        uint16_t message_length{8};
        uint8_t  message_type{};
        uint8_t  matching_unit{};
        uint32_t sequence_number{};
    };

    struct login_response
    {
        uint16_t start_of_message{0xBABA};
        uint16_t message_length{};
        uint8_t  message_type{};
        uint8_t  matching_unit{};
        uint32_t sequence_number{};
        char     login_response_status[1]{};
        char     login_response_text[60]{};
        uint8_t  no_unspecified_unit_replay{};
        uint32_t last_received_sequence_number{};
        uint8_t  number_of_units{0x00};
        uint8_t  number_of_param_groups{0x00};
    };    

    struct new_order
    {
        uint16_t start_of_message{0xBABA};
        uint16_t message_length{};
        uint8_t  message_type{};
        uint8_t  matching_unit{};
        uint32_t sequence_number{};
        char     client_order_id[20]{};
        char     side{};
        uint32_t order_qty{};
        uint8_t  number_of_new_order_bitfields{1};
        uint8_t  bitfield1{0x14};  // Price + OrdType
        uint8_t  bitfield2{0x41};  // Symbol + Capacity
        int64_t  price{};          
        char     ord_type{};       // '1' = Market, '2' = Limit
        char     symbol[8]{};     
        char     capacity{'C'};
    };

    struct cancel_order
    {
        uint16_t start_of_message{0xBABA};
        uint16_t message_length{};
        uint8_t  message_type{};
        uint8_t  matching_unit{};
        uint32_t sequence_number{};
        char     client_order_id[20]{};
        uint8_t  number_of_cancel_order_bitfields{1};
    };

    struct order_acknowledgement
    {
        uint16_t start_of_message{0xBABA};
        uint16_t message_length{};
        uint8_t  message_type{};
        uint8_t  matching_unit{0x00};
        uint32_t sequence_number{};
        uint64_t transaction_time{};    
        char     client_order_id[20]{};       
        uint64_t order_id{};            
        uint8_t  reserved_internal{0x00};
        uint8_t  num_return_bitfields{0};
    };

    struct order_rejected
    {
        uint16_t start_of_message{0xBABA};
        uint16_t message_length{};
        uint8_t  message_type{};
        uint8_t  matching_unit{0x00};
        uint32_t sequence_number{0x00};
        uint64_t transaction_time{};
        char     client_order_id[20]{};
        char     order_reject_reason{};
        char     text[60]{};
        uint8_t  reserved_internal{0x00};
        uint8_t  num_return_bitfields{0};
    };
    
    struct cancel_rejected {
        uint16_t start_of_message{START_OF_MESSAGE};
        uint16_t message_length{};
        uint8_t  message_type{};
        uint8_t  matching_unit{0x00};
        uint32_t sequence_number{0x00};
        uint64_t transaction_time{};
        char     cl_ord_id[20]{};
        char     cancel_reject_reason{};
        char     text[60]{};
        uint8_t  reserved_internal{0x00};
        uint8_t  num_return_bitfields{0};
    };

    #pragma pack(pop)
}
