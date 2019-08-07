//
// Created by zwg on 19-7-5.
//

#pragma once

#include <stdint.h>
#include <memory>
#include "protocol.hpp"

using namespace eosio;

template <typename T>
class Stat;

template <typename T>
class StatInfo {
    T msg;
    fc::time_point timeStamp;
    uint64_t sn;
    friend class Stat<T>;

public:
    StatInfo(T msg, fc::time_point t, uint64_t s):msg(msg), timeStamp(t), sn(s) {}
};

template <typename T>
class Stat {
    uint64_t microseconds;
    fc::time_point last;
    std::list<StatInfo<T> > messageList;
public:
    Stat(uint64_t seconds) {
        microseconds = seconds*1000000;
        last = fc::time_point::now();
    }
    void put(StatInfo<T>&& x) {
        messageList.push_back(x);
    }
    void statInfo(void) {
        auto now = fc::time_point::now();
        if((now - last).count() < 100000) return;
        last = now;
        while(!messageList.empty()) {
            if((now - messageList.begin()->timeStamp).count() > microseconds)
                messageList.pop_front();
            else break;
        }
        cout << "Recent " << microseconds
             << " microseconds: received "
             << messageList.size() << ", current total received:"
             << messageList.back().sn << endl;
    }
};
