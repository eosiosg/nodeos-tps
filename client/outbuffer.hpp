//
// Created by zwg on 19-6-28.
//

#pragma once

#include <array>
#include <list>
#include <boost/asio/buffer.hpp>
#include "protocol.hpp"

using namespace boost::asio;

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

class AsyncBufferPool {
public:
    uint8_t* newBuffer(std::size_t len) {
        return new uint8_t[len];
    }

    void deleteBuffer(uint8_t* buffer) {
        delete [] buffer;
    }
};

class BoostBufferPool {
    std::list<BOOST_ASIO_MUTABLE_BUFFER> v;
public:
    BOOST_ASIO_MUTABLE_BUFFER& add(uint8_t* buffer, std::size_t buffer_size) {
        v.push_back(boost::asio::buffer(buffer, buffer_size));
        return v.back();
    }

/*    void deleteBuffer(boost::asio::buffer* buffer) {
        delete [] buffer;
    }*/
};