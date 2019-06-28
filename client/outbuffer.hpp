//
// Created by zwg on 19-6-28.
//

#pragma once

#include <array>
#include "protocol.hpp"

class OutBuffer {
    uint8_t* pBuffer;
    uint32_t size;
    uint32_t used;
public:
    OutBuffer(uint32_t size):size(size),used(0) {
        pBuffer = new uint8_t[size];
    }

    virtual ~OutBuffer() {
        delete [] pBuffer;
    }

    template <typename T>
    bool toBuffer(const T& msg) {
        auto n = net_message(msg);
        uint32_t payload_size = fc::raw::pack_size(n);
        char *header = reinterpret_cast<char *>(&payload_size);
        size_t header_size = sizeof(payload_size);
        size_t msgLen = header_size + payload_size;
        if(msgLen > size - used)
            return false;
        fc::datastream<u_char *> ds(pBuffer + used, msgLen);
        ds.write(header, header_size);
        fc::raw::pack(ds, n);
        return true;
    }
};