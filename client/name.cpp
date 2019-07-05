//
// Created by zwg on 19-6-19.
//

#include <fc/variant.hpp>
#include <boost/algorithm/string.hpp>
#include <fc/exception/exception.hpp>
#include "protocol.hpp"
#include "types.hpp"

namespace eosio {
    namespace chain {

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

namespace fc {
    void to_variant(const eosio::name& c, variant& v) { v = std::string(c); }
    void from_variant(const variant& v, eosio::name& check) { check = v.get_string(); }
}