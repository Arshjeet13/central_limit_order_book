#pragma once

#include <unordered_map>
#include <string>
#include <array>
#include <cstring>

namespace db
{
    struct Credentials
    {
        char password[10]{};
        char session_sub_id[4]{};
    };

    inline const std::unordered_map<std::string, Credentials> users = {
        { "TST1", Credentials{ {'P', 'A', 'S', 'S', '0', '1', '2', '3', '4', '5'}, {'0', '0', '0', '1'} } },
        { "TST2", Credentials{ {'P', 'A', 'S', 'S', '1', '2', '3', '4', '5', '6'}, {'0', '0', '0', '2'} } },
    };

    inline bool validate(const char* username,
                        const char* password,
                        const char* session_sub_id){
        std::string key(username, 4);
        auto it = users.find(key);
        if (it == users.end())               return false;
        if (memcmp(it->second.password,       password,       10) != 0) return false;
        if (memcmp(it->second.session_sub_id, session_sub_id,  4) != 0) return false;
        return true;
    }
}