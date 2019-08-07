//
// Created by zwg on 19-6-28.
//

#pragma once

#include <array>
#include <list>
#include <boost/asio/buffer.hpp>
#include "protocol.hpp"
#include "exceptions.hpp"

using namespace boost::asio;


struct Buffer {
    uint8_t *pbuffer;
    std::size_t size;
    Buffer() = delete;
    Buffer(uint8_t* p, std::size_t s):pbuffer(p), size(s) { }
};

class AsyncBufferPool {
private:
    static AsyncBufferPool instance;
    static constexpr std::size_t allocSize = 1024;
    std::list<Buffer> empty;
    std::map<uint8_t*, std::size_t> full;
    AsyncBufferPool();
public:
    static AsyncBufferPool& Instance() {
        return instance;
    }
    virtual ~AsyncBufferPool();

    uint8_t* newBuffer(std::size_t len);

    void deleteBuffer(uint8_t* buffer);
};
