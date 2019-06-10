//
// Created by zwg on 19-6-6.
//

#ifndef NODEOS_TPS_OTHER_FC_H
#define NODEOS_TPS_OTHER_FC_H
#include <string>
#include <iostream>
#include <cstring>
#include <openssl/bn.h>
#include <stdexcept>
#include "base58.hpp"
#include <openssl/sha.h>
#include <openssl/ripemd.h>
using namespace std;

size_t from_hex( const string& hex_str, char* out_data, size_t out_data_len );

constexpr size_t const_strlen(const char* str);

uint32_t calculate_checksum(const void* pData, size_t size);

struct sha256 {
private:
    using HashType = uint64_t[4];
public:
    sha256() = default;
    uint64_t hash[4];
    sha256(const string& hexString);
    static pair<bool, size_t> serialize(void* pDstBuffer, const void* pData, size_t bufferSize);
    static pair<bool, size_t> deserialize(void* pDstData, const void* pBuffer, size_t bufferLength);
    bool operator==(const sha256& a1);
};

struct public_key {
    static string public_key_legacy_prefix;
    vector<char> storage;
    public_key() = default;
    public_key(const std::string& base58str);
    static pair<bool, size_t> serialize(void* pDstBuffer, const void* pData, size_t bufferSize);
    static pair<bool, size_t> deserialize(void* pDstData, const void* pBuffer, size_t bufferLength);
    bool operator==(const public_key& k);
};

#endif //NODEOS_TPS_OTHER_FC_H
