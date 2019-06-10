//
// Created by zwg on 19-6-7.
//

#include "base58.hpp"

std::pair<bool, std::vector<char>> from_base58( const std::string& base58_str ) {
    std::vector<unsigned char> out;
    if( !DecodeBase58( base58_str.c_str(), out ) ) {
        std::vector<char> t;
        return std::make_pair(false, t);
    }
    return std::make_pair(true, std::vector<char>((const char*)out.data(), ((const char*)out.data())+out.size()));
}

std::string to_base58( const char* d, size_t s ) {
    return EncodeBase58( (const unsigned char*)d, (const unsigned char*)d+s ).c_str();
}

uint32_t calculate_checksum(const void* pData, size_t size) {
    RIPEMD160_CTX ctx;
    RIPEMD160_Init(&ctx);
    uint32_t _hash[5];
    RIPEMD160_Update(&ctx, (const char *)pData, size);
    RIPEMD160_Final((unsigned char*)_hash, &ctx);
    return _hash[0];
}

std::string to_base58( const std::vector<char>& d )
{
    if( d.size() )
        return to_base58( d.data(), d.size() );
    return std::string();
}