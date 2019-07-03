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
    fc::time_point start;
    fc::time_point end;
    static constexpr std::size_t allocSize = 1024*32;
    std::list<Buffer> empty;
    std::map<uint8_t*, std::size_t> full;
public:
    AsyncBufferPool() {
        for(int i = 0; i < 30; i ++) {
            auto p = new uint8_t[allocSize];
            empty.emplace_back(Buffer(p, allocSize));
        }
    }
    ~AsyncBufferPool(){
        for(auto&x: empty) delete [] x.pbuffer;
        for(auto&x: full) delete [] x.first;
        empty.clear();
        full.clear();
    }

    uint8_t* newBuffer(std::size_t len) {
        start = fc::time_point::now();
        uint8_t* ret = nullptr;
        if(len <= allocSize) {
            if(!empty.empty()) {
                auto& f = empty.front();
                full[f.pbuffer] = f.size;
                ret = f.pbuffer;
                empty.pop_front();
            } else {
                if(full.size() > 3000) EOS_THROW(eosio::chain::new_buffer_exception, "full.size() > 3000");
                if(full.size() > 300) cout << "full.size() > 300" << endl;
                auto p = new uint8_t[allocSize];
                full[p] = allocSize;
                ret = p;
            }
        } else EOS_THROW(eosio::chain::new_buffer_exception, "newBuffer len > allocSize");
        end = fc::time_point::now();
        cout << "NewBufferTimeCost:" << (end-start).count() << endl;
        return ret;
    }

    void deleteBuffer(uint8_t* buffer) {
        start = fc::time_point::now();
        auto iter = full.find(buffer);
        auto size = iter->second;
        if(iter != full.end()) {
            full.erase(iter);
            empty.emplace_back(Buffer(buffer, size));
        } else EOS_THROW(eosio::chain::delete_buffer_exception, "Can not find pointer in pool.");
        end = fc::time_point::now();
        cout << "DeleteBufferTimeCost:" << (end-start).count() << endl;
    }
};
