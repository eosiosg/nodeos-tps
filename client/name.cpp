//
// Created by zwg on 19-6-19.
//

#include <fc/variant.hpp>
#include <boost/algorithm/string.hpp>
#include <fc/exception/exception.hpp>
#include "protocol.hpp"

namespace eosio {

    static constexpr uint64_t char_to_symbol(char c)
    {
        if (c>='a' && c<='z')
            return (c-'a')+6;
        if (c>='1' && c<='5')
            return (c-'1')+1;
        return 0;
    }

    static constexpr uint64_t string_to_name(const char* str)
    {
        uint64_t name = 0;
        int i = 0;
        for (; str[i] && i<12; ++i) {
            // NOTE: char_to_symbol() returns char type, and without this explicit
            // expansion to uint64 type, the compilation fails at the point of usage
            // of string_to_name(), where the usage requires constant (compile time) expression.
            name |= (char_to_symbol(str[i]) & 0x1f) << (64-5*(i+1));
        }

        // The for-loop encoded up to 60 high bits into uint64 'name' variable,
        // if (strlen(str) > 12) then encode str[12] into the low (remaining)
        // 4 bits of 'name'
        if (i==12)
            name |= char_to_symbol(str[12]) & 0x0F;
        return name;
    }

    void name::set(const char* str)
    {
        const auto len = strnlen(str, 14);
        EOS_ASSERT(len<=13, name_type_exception, "Name is longer than 13 characters (${name}) ", ("name", std::string(str)));
        value = string_to_name(str);
        EOS_ASSERT(to_string()==std::string(str), name_type_exception,
                "Name not properly normalized (name: ${name}, normalized: ${normalized}) ",
                ("name", std::string(str))("normalized", to_string()));
    }

    // keep in sync with name::to_string() in contract definition for name
    name::operator std::string() const
    {
        static const char* charmap = ".12345abcdefghijklmnopqrstuvwxyz";

        std::string str(13, '.');

        uint64_t tmp = value;
        for (uint32_t i = 0; i<=12; ++i) {
            char c = charmap[tmp & (i==0 ? 0x0f : 0x1f)];
            str[12-i] = c;
            tmp >>= (i==0 ? 4 : 5);
        }

        boost::algorithm::trim_right_if(str, [](char c) { return c=='.'; });
        return str;
    }

}
