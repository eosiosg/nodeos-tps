//
// Created by zwg on 19-7-6.
//

#pragma once

#include <boost/asio.hpp>
#include <memory>
using boost::asio::ip::udp;
using namespace std;

class Grafana{
    udp::socket s;
    boost::asio::io_service& ioc;
    udp::resolver resolver;
    udp::resolver::query query{udp::v4(), "127.0.0.1", "8089"};
    udp::resolver::iterator iterator;
    const string ip;
    const string port;
public:
    Grafana(boost::asio::io_context& ioc, const char* ip, const char* port):
    ioc(ioc),ip(ip), port(port), s(ioc, udp::endpoint(udp::v4(), 0)), resolver(ioc) {
        this->query = udp::resolver::query{udp::v4(), this->ip.c_str(), this->port.c_str()};
        iterator = resolver.resolve(query);
    }
    Grafana(boost::asio::io_context& ioc):
    ioc(ioc),ip("127.0.0.1"), port("8089"), s(ioc, udp::endpoint(udp::v4(), 0)), resolver(ioc) {
        iterator = resolver.resolve(query);
    }
    void send(const char* buffer, std::size_t len) {
        if(!s.is_open()) {
            cerr << "Warning: s.is_open() is false" << endl;
            this->query = udp::resolver::query{udp::v4(), this->ip.c_str(), this->port.c_str()};
            this->s = udp::socket(ioc, udp::endpoint(udp::v4(), 0));
            iterator = resolver.resolve(query);
        }
        s.send_to(boost::asio::buffer(buffer, len), *iterator);
    }
};

