//
// Created by zwg on 19-6-6.
//

#ifndef NODEOS_TPS_OTHER_FC_H
#define NODEOS_TPS_OTHER_FC_H
#include <string>
#include <array>
#include <iostream>
#include <cstring>
#include <openssl/bn.h>
#include <stdexcept>
#include "base58.hpp"
#include <openssl/sha.h>
#include <openssl/ripemd.h>
#include <secp256k1.h>
using namespace std;

size_t from_hex( const string& hex_str, char* out_data, size_t out_data_len );

constexpr size_t const_strlen(const char* str);

int extended_nonce_function( unsigned char *nonce32, const unsigned char *msg32,
                                    const unsigned char *key32, unsigned int attempt,
                                    const void *data );

class sha256 {
private:
    using HashType = uint64_t[4];
public:
    sha256() = default;
    uint64_t _hash[4];
    sha256(const string& hexString);
    static pair<bool, size_t> serialize(void* pDstBuffer, const void* pData, size_t bufferSize);
    static pair<bool, size_t> deserialize(void* pDstData, const void* pBuffer, size_t bufferLength);
    bool operator==(const sha256& a1);
    sha256& operator=(const sha256& a) {
        memcpy(_hash, a._hash, sizeof(HashType));
        return *this;
    }
    char* data()const { return (char*)&_hash[0]; }
    template<typename T>
    static sha256 hash(const T& t) {
        sha256 h;
        SHA256_CTX ctx;
        SHA256_Init(&ctx);
        SHA256_Update(&ctx, &t, sizeof(T));
        SHA256_Final((uint8_t*)h._hash, &ctx);
        return h;
    }

    static sha256 hash(const void*p, size_t len) {
        sha256 h;
        SHA256_CTX ctx;
        SHA256_Init(&ctx);
        SHA256_Update(&ctx, p, len);
        SHA256_Final((uint8_t*)h._hash, &ctx);
        return h;
    }
};

pair<bool, sha256> from_wif( const string& wif_key );

class public_key {
public:
    static string public_key_legacy_prefix;
    vector<char> storage;
    public_key() = default;
    public_key(const std::string& base58str);
    static pair<bool, size_t> serialize(void* pDstBuffer, const void* pData, size_t bufferSize);
    static pair<bool, size_t> deserialize(void* pDstData, const void* pBuffer, size_t bufferLength);
    bool operator==(const public_key& k);
};


class signature {
    std::array<unsigned char, 65> _data;
public:
    std::array<unsigned char, 65>::const_pointer data(void) const {
        return _data.data();
    }

    std::array<unsigned char, 65>::iterator begin(void) {
        return _data.begin();
    }

    static pair<bool, size_t> serialize(void* pDstBuffer, const void* pData, size_t bufferSize) {
        if(bufferSize < 66) return make_pair(false, 0);
        ((unsigned char*)pDstBuffer)[0] = 0; // 当前用的是K1,版本号写死为0
        memcpy((unsigned char*)pDstBuffer + 1, ((const signature*)pData)->_data.data(), 65);
        return make_pair(true, 66);
    }

    static pair<bool, size_t> deserialize(void* pDstData, const void* pBuffer, size_t bufferLength) {
        if(bufferLength < 66) return make_pair(false, 0);
        //pBuffer[0] is version info, just drop it.
        memcpy(((signature*)pDstData)[0]._data.data(), (const char*)pBuffer + 1, 65);
        return make_pair(true, 66);
    }

    bool operator==(const signature& sig) {
        return sig._data == sig._data;
    }
};

class private_key {

    const secp256k1_context_t* _get_context() const {
        static secp256k1_context_t* ctx = secp256k1_context_create(SECP256K1_CONTEXT_VERIFY | SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_RANGEPROOF | SECP256K1_CONTEXT_COMMIT );
        return ctx;
    }

    static pair<bool, sha256> parse_base58(const string& base58str)
    {
        const auto pivot = base58str.find('_');

        if (pivot == std::string::npos) {
            return from_wif(base58str);
        } else {
            // TODO: ?? What scene goes here?
            cerr << "Not implemented \"sha256 parse_base58\"." << endl;
            return make_pair(false, sha256());
        }
    }
    sha256 _key;
public:
    private_key(const std::string& base58str) {
        auto rt = parse_base58(base58str);
        if(!rt.first) {
            cerr << "parse base 58 failed." << endl;
            throw "Parse base 58 failed in private_key()";
        }
        _key = rt.second;
    }

    static bool is_canonical( const signature& c ) {
        return !(c.data()[1] & 0x80) && !(c.data()[1] == 0 && !(c.data()[2] & 0x80))
               && !(c.data()[33] & 0x80) && !(c.data()[33] == 0 && !(c.data()[34] & 0x80));
    }

    signature sign(const sha256& digest, bool require_canonical = true) const {
        signature result;
        int recid;
        unsigned int counter = 0;
        do { secp256k1_ecdsa_sign_compact( _get_context(), (unsigned char*) digest.data(),
                    (unsigned char*) result.begin() + 1, (unsigned char*) _key.data(),
                    extended_nonce_function, &counter, &recid );
        } while( require_canonical && !is_canonical( result ) );
        result.begin()[0] = 27 + 4 + recid;
        return result;
    }
};



#endif //NODEOS_TPS_OTHER_FC_H
