//
// Created by zwg on 19-6-21.
//

#pragma once
#include <string>
#include <fc/reflect/reflect.hpp>
#include <iosfwd>

namespace eosio { 
    namespace chain {
        using std::string;
        static constexpr uint64_t char_to_symbol(char c)
        {
            if (c>='a' && c<='z')
                return (c-'a')+6;
            if (c>='1' && c<='5')
                return (c-'1')+1;
            return 0;
        }
        constexpr uint64_t string_to_name(const char* str) {
            uint64_t name = 0;
            int i = 0;
            for (; str[i] && i<12; ++i) name |= (char_to_symbol(str[i]) & 0x1f) << (64-5*(i+1));
            if (i==12) name |= char_to_symbol(str[12]) & 0x0F;
            return name;
        }
        struct name {
            uint64_t value = 0;
            bool empty()const { return 0 == value; }
            bool good()const  { return !empty();   }

            name( const char* str )   { set(str);           }
            name( const string& str ) { set( str.c_str() ); }

            void set( const char* str );

            template<typename T>
            name( T v ):value(v){}
            name(){}

            explicit operator string()const;

            string to_string() const { return string(*this); }

            name& operator=( uint64_t v ) {
                value = v;
                return *this;
            }

            name& operator=( const string& n ) {
                value = name(n).value;
                return *this;
            }
            name& operator=( const char* n ) {
                value = name(n).value;
                return *this;
            }

            friend std::ostream& operator << ( std::ostream& out, const name& n ) {
                return out << string(n);
            }

            friend bool operator < ( const name& a, const name& b ) { return a.value < b.value; }
            friend bool operator <= ( const name& a, const name& b ) { return a.value <= b.value; }
            friend bool operator > ( const name& a, const name& b ) { return a.value > b.value; }
            friend bool operator >=( const name& a, const name& b ) { return a.value >= b.value; }
            friend bool operator == ( const name& a, const name& b ) { return a.value == b.value; }

            friend bool operator == ( const name& a, uint64_t b ) { return a.value == b; }
            friend bool operator != ( const name& a, uint64_t b ) { return a.value != b; }

            friend bool operator != ( const name& a, const name& b ) { return a.value != b.value; }

            explicit  operator bool()const            { return value; }
            explicit operator uint64_t()const        { return value; }
            explicit operator unsigned __int128()const       { return value; }
        };
    } 
} // eosio::chain

namespace std {
    template<> struct hash<eosio::chain::name> : private hash<uint64_t> {
        typedef eosio::chain::name argument_type;
        typedef typename hash<uint64_t>::result_type result_type;
        result_type operator()(const argument_type& name) const noexcept
        {
            return hash<uint64_t>::operator()(name.value);
        }
    };
}

namespace fc {
    class variant;
    void to_variant(const eosio::chain::name& c, fc::variant& v);
    void from_variant(const fc::variant& v, eosio::chain::name& check);
} // fc


FC_REFLECT( eosio::chain::name, (value) )
