//
// Created by zwg on 19-7-6.
//

#pragma once

#include <boost/asio.hpp>
#include <memory>
#include <string>
#include <iostream>
#include "iocontext.hpp"
using boost::asio::ip::udp;
using namespace std;

class Grafana{
    udp::socket s;
    udp::resolver resolver;
    string ip;
    string port;
    udp::resolver::query query;
    udp::resolver::iterator iterator;
public:
    Grafana(const string ip = string("127.0.0.1"), const string port = string("8089")):
    ip(ip), port(port), resolver(IOC::app()),
    s(IOC::app(), udp::endpoint(udp::v4(), 0)), query{udp::v4(), ip, port}{
        iterator = resolver.resolve(query);
    }
    Grafana() = delete;
    Grafana(Grafana&&) = delete;
    void send(const char* buffer, std::size_t len) {
        if(!s.is_open()) {
            cerr << "Warning: s.is_open() is false" << endl;
            query = udp::resolver::query{udp::v4(), ip, port};
            s = udp::socket(IOC::app(), udp::endpoint(udp::v4(), 0));
            iterator = resolver.resolve(query);
        }
        s.send_to(boost::asio::buffer(buffer, len), *iterator);
    }
};

