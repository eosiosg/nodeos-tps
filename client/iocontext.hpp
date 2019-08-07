//
// Created by zwg on 19-7-11.
//

#pragma once
#include <boost/asio/io_context.hpp>
class IOC {
private:
    static boost::asio::io_context ioc;
public:
    IOC() = delete;
    IOC(IOC&) = delete;
    IOC(IOC&&) = delete;
    static boost::asio::io_context& app() {
        return ioc;
    }
};