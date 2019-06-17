//
// Created by zwg on 19-6-13.
//

#ifndef NODEOS_TPS_CLIENT_HPP
#define NODEOS_TPS_CLIENT_HPP

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
#include "protocol.hpp"
#include "fake_data.hpp"

using boost::asio::ip::tcp;
using namespace eosio;
using namespace std;

constexpr auto     def_send_buffer_size_mb = 4;
constexpr auto     def_send_buffer_size = 1024*1024*def_send_buffer_size_mb;
constexpr auto     def_max_write_queue_size = def_send_buffer_size*10;
constexpr auto     message_header_size = 4;
constexpr boost::asio::chrono::milliseconds def_read_delay_for_full_write_queue{100};
constexpr auto     def_max_reads_in_flight = 1000;
constexpr auto     def_max_trx_in_progress_size = 100*1024*1024; // 100 MB
constexpr auto     def_max_clients = 25; // 0 for unlimited clients
constexpr auto     def_max_nodes_per_host = 1;
constexpr auto     def_conn_retry_wait = 30;
constexpr auto     def_txn_expire_wait = std::chrono::seconds(3);
constexpr auto     def_resp_expected_wait = std::chrono::seconds(5);
constexpr auto     def_sync_fetch_span = 100;

using signed_block_ptr = std::shared_ptr<signed_block>;
using packed_transaction_ptr = std::shared_ptr<packed_transaction>;

#define EOS_ASSERT( expr, exc_type, FORMAT, ... )                \
   FC_MULTILINE_MACRO_BEGIN                                           \
   if( !(expr) )                                                      \
      FC_THROW_EXCEPTION( exc_type, FORMAT, __VA_ARGS__ );            \
   FC_MULTILINE_MACRO_END

FC_DECLARE_EXCEPTION( chain_exception, 3000000, "blockchain exception" )
FC_DECLARE_DERIVED_EXCEPTION( plugin_exception, chain_exception, 3110000, "Plugin exception" )
FC_DECLARE_DERIVED_EXCEPTION( plugin_config_exception, plugin_exception, 3110006, "Incorrect plugin configuration" )


class Client {
    tcp::socket _socket;
    tcp::resolver _resolver;
    boost::asio::steady_timer _timer;
    fc::message_buffer<1024*1024> _messageBuffer;
    fc::optional<std::size_t> _outStandingReadBytes; //下次需要读取的字节数
    u_char *buffer;

public:
    Client(boost::asio::io_context& ioc, const char* host, const char* port);
    void OnConnect(boost::system::error_code ec, tcp::endpoint endpoint);
    bool processNextMessage(uint32_t messageLen);
    void handleMessage(const handshake_message& msg);
    void handleMessage(const chain_size_message& msg);
    void handleMessage(const go_away_message& msg);
    void handleMessage(const time_message& msg) ;
    void handleMessage(const notice_message& msg);
    void handleMessage(const request_message& msg);
    void handleMessage(const sync_request_message& msg);
    void handleMessage(const signed_block_ptr& msg);
    void handleMessage(const packed_transaction_ptr& msg);
    void handleMessage(const response_p2p_message& msg);
    void handleMessage(const request_p2p_message& msg);
    void StartReadMessage();
    void StartSendTimeMessage();
    void StartHandshakeMessage();
    void DoSendTimeMessage();
    void sendHandshakeMessage(handshake_message && msg);
    void OnResolve(boost::system::error_code ec, tcp::resolver::results_type endpoints);
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


#endif //NODEOS_TPS_CLIENT_HPP
