//
// Created by zwg on 19-7-11.
//
#pragma once

#include <list>
#include "client.hpp"
#include "iocontext.hpp"
#include <chrono>
#include <memory>
using namespace std;

class ClientPool {
    list<std::unique_ptr<Client>> l;
    boost::asio::steady_timer loopTimer{IOC::app()};
    uint64_t sleepMS;
    void checkClient(){
        for(auto&c: l) {
            if(c->CanCheckStatus()&&!c->IsConnect())
                c->Reconnect();
        }
        loopTimer.expires_after(std::chrono::milliseconds(sleepMS));
        loopTimer.async_wait(std::bind(&ClientPool::checkClient, this));
    }
public:
    ClientPool(uint64_t ms = 2000):sleepMS(ms) {
        loopTimer.expires_after(std::chrono::milliseconds(sleepMS));
        loopTimer.async_wait(std::bind(&ClientPool::checkClient, this));
    }
    void newClient(std::unique_ptr<Client>&& pClient) {
        l.emplace_back(std::move(pClient));
    }
};

