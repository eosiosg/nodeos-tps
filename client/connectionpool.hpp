//
// Created by zwg on 19-6-27.
//

#pragma once

#include <set>
#include <vector>
#include <string>
#include <memory>
#include <boost/asio/ip/tcp.hpp>
class ConnectionPool {
    std::vector<std::string> hosts;
    std::vector<std::string> ports;
    std::unique_ptr<boost::asio::io_context> pIoc;
    std::vector<std::unique_ptr<boost::asio::ip::tcp>> sockets;
    ConnectionPool(
            std::vector<std::string>&& hosts,
            std::vector<std::string>&& ports,
            boost::asio::io_context&& ioc) {

}
};