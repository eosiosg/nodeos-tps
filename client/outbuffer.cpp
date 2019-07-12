//
// Created by zwg on 19-7-11.
//

#include "outbuffer.hpp"

AsyncBufferPool AsyncBufferPool::instance;

AsyncBufferPool::AsyncBufferPool() {
    for(int i = 0; i < 300; i ++) {
        auto p = new uint8_t[allocSize];
        empty.emplace_back(Buffer(p, allocSize));
    }
}

AsyncBufferPool::~AsyncBufferPool(){
    for(auto&x: empty) delete [] x.pbuffer;
    for(auto&x: full) delete [] x.first;
    empty.clear();
    full.clear();
}

uint8_t* AsyncBufferPool::newBuffer(std::size_t len) {
    uint8_t* ret = nullptr;
    if(len <= allocSize) {
        if(!empty.empty()) {
            auto& f = empty.front();
            full[f.pbuffer] = f.size;
            ret = f.pbuffer;
            empty.pop_front();
        } else {
            if(full.size() > 3000) EOS_THROW(eosio::chain::new_buffer_exception, "full.size() > 3000");
            auto p = new uint8_t[allocSize];
            full[p] = allocSize;
            ret = p;
        }
    } else EOS_THROW(eosio::chain::new_buffer_exception, "newBuffer len > allocSize");
    return ret;
}

void AsyncBufferPool::deleteBuffer(uint8_t* buffer) {
    auto iter = full.find(buffer);
    auto size = iter->second;
    if(iter != full.end()) {
        full.erase(iter);
        empty.emplace_back(Buffer(buffer, size));
    } else EOS_THROW(eosio::chain::delete_buffer_exception, "Can not find pointer in pool.");
}