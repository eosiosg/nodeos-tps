//
// Created by zwg on 19-6-13.
//

#pragma once
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <fc/network/message_buffer.hpp>
#include <fc/exception/exception.hpp>
#include <memory>
#include <iostream>
#include <map>
#include <functional>
#include <tuple>
#include <chrono>
#include <deque>
#include <utility>
#include <cstdlib>
#include <ctime>
#include <boost/asio/high_resolution_timer.hpp>

#include "protocol.hpp"
#include "fake_data.hpp"
#include "eosio.token.api.hpp"
#include "eosio.system.abi.hpp"
#include "abi_def.hpp"
#include "abi_serializer.hpp"
#include "symbol.hpp"
#include "asset.hpp"
#include "authority.hpp"
#include "contract_types.hpp"
#include "eosio.token.wast.hpp"
#include "outbuffer.hpp"
#include "iocontext.hpp"

using boost::asio::ip::tcp;
using namespace std;
using namespace eosio;

using signed_block_ptr = std::shared_ptr<signed_block>;
using packed_transaction_ptr = std::shared_ptr<packed_transaction>;

constexpr auto     def_send_buffer_size_mb = 4;
constexpr auto     def_send_buffer_size = 1024*1024*def_send_buffer_size_mb;
constexpr auto     message_header_size = 4;

struct OutQueue {
    std::list<Buffer> _q;
    uint8_t *pTemp= nullptr;
    std::size_t wIndex = 0;
    static constexpr std::size_t allocSize = 1024;
    AsyncBufferPool& pool;
    uint64_t microsEmptyWaitTime;
    OutQueue() = delete;
    OutQueue(AsyncBufferPool& pool, uint64_t microsEmptyWaitTime = 2000):pool(pool) {
        this->microsEmptyWaitTime = microsEmptyWaitTime;
        pTemp = pool.newBuffer(allocSize);
    }
    ~OutQueue() {
        pool.deleteBuffer(pTemp);
    }
    std::size_t size() {
        return _q.size();
    }
    Buffer& front() {
        return _q.front();
    }
    void push_back(const Buffer& b) {
        _q.push_back(b);
    }
    bool empty() {
        return _q.empty();
    }
    void pop_front() {
        _q.pop_front();
    }
};

class Client {
protected:
    tcp::socket _socket{IOC::app()};
    tcp::resolver _resolver{IOC::app()};
    boost::asio::steady_timer timerMakePeerSync{IOC::app()};
    boost::asio::steady_timer timerSendTimeMessage{IOC::app()};
    boost::asio::steady_timer timerOutQueue{IOC::app()};
    fc::message_buffer<1024*1024*20> _messageBuffer;
    fc::optional<std::size_t> _outStandingReadBytes; //下次需要读取的字节数
    AsyncBufferPool& bufferPool;
    string host;
    string port;
    bool connect{false};
    bool canCheckStatus{false};
    ostream &output = cout;
    OutQueue outQueue{bufferPool};
private:
    virtual void makePeerSync(void);
    virtual void DoSendoutData(void);
    virtual void StartSendoutData(void);
public:
    virtual OutQueue* GetQueuePointer() {
        return &outQueue;
    }
    virtual void Reconnect(void);
    inline bool CanCheckStatus() {return canCheckStatus;}
    inline bool IsConnect() {return connect;}
    Client(const string& host, const string& port):bufferPool{AsyncBufferPool::Instance()} {
        this->host = host;
        this->port = port;
        Reconnect();
    }

    virtual void OnConnect(boost::system::error_code ec, tcp::endpoint endpoint);
    virtual bool processNextMessage(uint32_t messageLen);

    virtual void handleMessage(const notice_message& msg);
    virtual void handleMessage(const signed_block_ptr& msg){ }

    template<typename T>
    void handleMessage(const T& msg){}

    virtual void StartReadMessage();
    virtual void StartSendTimeMessage();
    virtual void StartHandshakeMessage();
    virtual void DoSendTimeMessage();
    virtual void sendHandshakeMessage(handshake_message && msg);
    virtual void sendMessage(packed_transaction&& msg);

    template <typename T>
    void sendMessage(T && msg) {
        net_message netMsg(msg);
        int32_t payload_size = fc::raw::pack_size(netMsg);
        char* header = reinterpret_cast<char*>(&payload_size);
        size_t header_size = sizeof(payload_size);
        size_t messageLen = header_size + payload_size;
        uint8_t* buffer = bufferPool.newBuffer(messageLen);
        fc::datastream<uint8_t*> ds(buffer, messageLen);
        ds.write(header, header_size);
        fc::raw::pack(ds, netMsg);
        outQueue.push_back(Buffer(buffer, messageLen));
    }

    virtual void OnResolve(boost::system::error_code ec, tcp::resolver::results_type endpoints);
    virtual ~Client(){ }
};

struct MsgHandler : public fc::visitor<void> {
    Client* _pCli;
    MsgHandler(Client* p);
    void operator()( signed_block&& msg ) {
        _pCli->handleMessage(std::make_shared<signed_block>( std::move( msg ) ) );
    }
    void operator()( packed_transaction&& msg ) {
        _pCli->handleMessage(std::make_shared<packed_transaction>( std::move( msg ) ) );
    }

    /* 下面四个函数实际上永远不可能调用到，但是不定义编译器会报错*/
    void operator()( const signed_block& msg ) const {
        EOS_ASSERT( false, plugin_config_exception, "operator()(signed_block&&) should be called" );
    }
    void operator()( signed_block& msg ) const {
        EOS_ASSERT( false, plugin_config_exception, "operator()(signed_block&&) should be called" );
    }
    void operator()( const packed_transaction& msg ) const {
        EOS_ASSERT( false, plugin_config_exception, "operator()(packed_transaction&&) should be called" );
    }
    void operator()( packed_transaction& msg ) const {
        EOS_ASSERT( false, plugin_config_exception, "operator()(packed_transaction&&) should be called" );
    }

    template <typename T>
    void operator()( T&& msg ) const {_pCli->handleMessage(std::forward<T>(msg));}
};