//
// Created by zwg on 19-6-6.
//
#include "other_fc.hpp"

uint8_t from_hex( char c ) {
    if( c >= '0' && c <= '9' )
        return c - '0';
    else if( c >= 'a' && c <= 'f' )
        return c - 'a' + 10;
    else if( c >= 'A' && c <= 'F' )
        return c - 'A' + 10;
    else cerr << "Invalid char." << endl;
    return 0;
}

size_t from_hex( const string& hex_str, char* out_data, size_t out_data_len ) {
    string::const_iterator i = hex_str.begin();
    uint8_t* out_pos = (uint8_t*)out_data;
    uint8_t* out_end = out_pos + out_data_len;
    while( i != hex_str.end() && out_end != out_pos ) {
        *out_pos = from_hex( *i ) << 4;
        ++i;
        if( i != hex_str.end() )  {
            *out_pos |= from_hex( *i );
            ++i;
        }
        ++out_pos;
    }
    return out_pos - (uint8_t*)out_data;
}


string public_key::public_key_legacy_prefix = "EOS";

constexpr size_t const_strlen(const char* str) {
    int i = 0;
    while(*(str+i) != '\0')
        i++;
    return i;
}


sha256::sha256(const string& hexString) {
    from_hex(hexString, (char*)&hash, sizeof(hash));
}

pair<bool, size_t> sha256::serialize(void* pDstBuffer, const void* pData, size_t bufferSize) {
    if(bufferSize < sizeof(HashType)) return make_pair(false, 0);
    memcpy(pDstBuffer, ((sha256*)pData)->hash, sizeof(HashType));
    return make_pair(true, sizeof(HashType));
}

pair<bool, size_t> sha256::deserialize(void* pDstData, const void* pBuffer, size_t bufferLength) {
    if(sizeof(HashType) > bufferLength) return make_pair(false, 0);
    memcpy(((sha256*)pDstData)->hash, pBuffer, sizeof(HashType));
    return make_pair(true, sizeof(HashType));
}

bool sha256::operator==(const sha256& a1) {
    for(auto i = 0; i < 4; i++) {
        if(a1.hash[i] != hash[i])
            return false;
    }
    return true;
}

public_key::public_key(const std::string& base58str) {
    auto sub_str = base58str.substr(const_strlen(public_key_legacy_prefix.data()));
    auto ret = from_base58(sub_str);
    if(!ret.first) {
        cerr << "Invalid base58 string(" << base58str << ")." << endl;
        throw "Invalid base58 string";
    }
    auto checksumIndex = ret.second.size() - sizeof(uint32_t);
    auto checksum = *((uint32_t*) &ret.second[checksumIndex]);
    auto calculateChecksum = calculate_checksum(ret.second.data(), checksumIndex);
    if(checksum != calculateChecksum) {
        cerr << "Invalid checksum." << endl;
        throw "Invalid checksum.";
    }
    storage = vector<char>(ret.second.data(), ret.second.data() + checksumIndex);
}

pair<bool, size_t> public_key::serialize(void* pDstBuffer, const void* pData, size_t bufferSize) {
    auto len = ((vector<char>*)pData)->size(); //实际上是定长33Bytes
    if(len + 1 > bufferSize) return make_pair(false, 0); //多写一个字节的版本信息
    memset(pDstBuffer, 0, 1); // 有一个字节的版本信息，写死为0
    memcpy((char*)pDstBuffer + 1, ((vector<char>*)pData)->data(), len);
    return make_pair(true, len + 1);
}

pair<bool, size_t> public_key::deserialize(void* pDstData, const void* pBuffer, size_t bufferLength) {
    if(bufferLength < 34) return make_pair(false, 0);
    ((vector<char>*)pDstData)->resize(33);
    auto p = ((vector<char>*)pDstData)->data();
    memcpy(p, (const char*)pBuffer + 1, 33); // 跳过第一个字节（版本信息）
    return make_pair(true, 34);
}

bool public_key::operator==(const public_key& k) {
    for(auto i=0;i <33;i++) {
        if(storage[i] != k.storage[i]) return false;
    }
    return true;
}