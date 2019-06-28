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
#include "eosio.token.api.hpp"
#include "eosio.system.abi.hpp"
#include "abi_def.hpp"
#include "abi_serializer.hpp"
#include "symbol.hpp"
#include "asset.hpp"
#include "authority.hpp"
#include "contract_types.hpp"
#include "eosio.token.wast.hpp"
#include <boost/asio/high_resolution_timer.hpp>
#include "outbuffer.hpp"

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

// 测试的数据信息存储在此
struct TestInfo {
    fc::microseconds abi_serializer_max_time{150000000};
    chain_id_type chain_id;
    block_id_type head_block_id;
    action act_a_to_b;
    action act_b_to_a;
    name user1;
    name user2;
    fc::crypto::private_key user1PK;
    fc::crypto::private_key user2PK;
    name contractName;
    string tokenName; //BOS, EOS, SYS, ......
    uint64_t timer_timeout;
    unsigned batch;
    boost::asio::steady_timer _timer;
    TestInfo(boost::asio::io_context& ioc):_timer(ioc){};
    void update(const signed_block &msg) {
        this->head_block_id = msg.previous;
    }

};

class OutputGuard {
    string thing;
    fc::time_point start;
    ostream & out;
    OutputGuard() = delete;
public:
    OutputGuard(ostream& out, string thing):out(out),thing(thing) {
        start = fc::time_point::now();
        out << "Begin do \"" << thing << "\"" << endl;
    }
    virtual ~OutputGuard() {
        out << "End do \"" << thing
            << "\"," << "time cost "
            << (fc::time_point::now() - start).count()/1000.0
            << " ms" << endl;
    }
};


class Client : public std::enable_shared_from_this<Client>{
    TestInfo testInfo;
    tcp::socket _socket;
    tcp::resolver _resolver;
    boost::asio::steady_timer _timer;
    fc::message_buffer<1024*1024*20> _messageBuffer;
    fc::optional<std::size_t> _outStandingReadBytes; //下次需要读取的字节数
    u_char *buffer;
    AsyncBufferPool bufferPool;
    BoostBufferPool boostBufferPool;
    ostream &output = cout;
    string host;
    string port;
    boost::asio::io_context& ioc;
    void makePeerSync(void);
    void performanceTest(void);
    //void createTestAccounts(const string& init_name, const string& init_priv_key, const string& core_symbol);
    void executeTest(void);
    void startGeneration(const string& salt, const uint64_t& period, const uint64_t& batch_size);
    void sendTransferTransaction();
    void reConnect(void) {
        _resolver.async_resolve(tcp::v4(), host, port,
                std::bind(&Client::OnResolve, this,
                        std::placeholders::_1,
                        std::placeholders::_2));
    }
public:
    Client(boost::asio::io_context& ioc,
           const char* host,
           const char* port,
           const char* chain_id,
           const char* user1,
           const char* user1PK,
           const char* user2,
           const char* user2PK,
           const char* tokenName,
           const char* contractName);
    virtual ~Client(void);
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

    void handleMessage(const pbft_prepare &msg);
    void handleMessage(const pbft_commit &msg);
    void handleMessage(const pbft_view_change &msg);
    void handleMessage(const pbft_new_view &msg);
    void handleMessage(const pbft_checkpoint &msg);
    void handleMessage(const pbft_stable_checkpoint &msg);
    void handleMessage(const checkpoint_request_message &msg);
    void handleMessage(const compressed_pbft_message &msg);

    void StartReadMessage();
    void StartSendTimeMessage();
    void StartHandshakeMessage();
    void DoSendTimeMessage();
    void sendHandshakeMessage(handshake_message && msg);
    template <typename T>
    void sendMessage(T && msg) {
        if(!_socket.is_open()) return;
        auto n = net_message(msg);
        uint32_t payload_size = fc::raw::pack_size(n);
        char *header = reinterpret_cast<char *>(&payload_size);
        size_t header_size = sizeof(payload_size);
        size_t buffer_size = header_size + payload_size;
        //cout << "buffer size:" << buffer_size << endl;
        uint8_t * buffer = bufferPool.newBuffer(buffer_size);
        fc::datastream<uint8_t *> ds(buffer, buffer_size);
        ds.write(header, header_size);
        fc::raw::pack(ds, n);
        _socket.async_write_some(
                boost::asio::buffer(buffer, buffer_size),
                [this, buffer](boost::system::error_code ec, std::size_t w){
                    bufferPool.deleteBuffer(buffer);
                    if(ec) {
                        cerr << "Error when asyn_write_some message." << endl;
                        cerr << ec.message() << endl;
                        _socket.close();
                        ioc.stop();
                        return;
                    }
                    //cout << "async write handshake_message successfully." << endl;
                });
    }
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
