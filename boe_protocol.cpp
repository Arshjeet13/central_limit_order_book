#include <cstdint>

namespace boe
{
    #pragma pack(push, 1)

    struct login_request
    {
        uint16_t start_of_message{0xBABA};
        uint16_t message_length{27}; // The message length does not include the 2 start_of_message bytes
        uint8_t  message_type{0x37};
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
        uint16_t message_length{8}; // The message length does not include the 2 start_of_message bytes
        uint8_t  message_type{0x37};
        uint8_t  matching_unit{};
        uint32_t sequence_number{};
    };

    // From section 2.4 : if no data has been sent in either direction for 1 second, 
    // both sides send heartbeats. If the server receives nothing from you for 5 seconds
    // it sends a Logout and closes the connection.
    struct client_hearbeat
    {
        uint16_t start_of_message{0xBABA};
        uint16_t message_length{8}; // The message length does not include the 2 start_of_message bytes
        uint8_t  message_type{0x37};
        uint8_t  matching_unit{};
        uint32_t sequence_number{};
    };

    struct login_response
    {
        uint16_t start_of_message{0xBABA};
        uint16_t message_length{}; // The message length does not include the 2 start_of_message bytes
        uint8_t  message_type{0x37};
        uint8_t  matching_unit{};
        uint32_t sequence_number{};
        char login_response_status[1]{};
        char login_response_text[60]{};
        uint8_t no_unspecified_unit_replay{};
        uint32_t last_received_sequence_number{};
        uint8_t number_of_units{0x00};
        uint8_t  number_of_param_groups{0x00};
    };    

    #pragma pack(pop)
}
