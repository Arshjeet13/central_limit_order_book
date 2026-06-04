#include <cstdint>

namespace boe
{
    #pragma pack(push, 1)

    struct login_request
    {
        uint16_t start_of_message{0xBABA};
        uint16_t message_length{27}; // The message length does not include the start of message byte
        uint8_t  message_type{0x37};
        uint8_t  matching_unit{};
        uint32_t sequence_number{};
        char     session_sub_id[4]{};
        char     username[4]{};
        char     password[10]{};
        uint8_t  number_of_param_groups{0};
    };

    #pragma pack(pop)
}
