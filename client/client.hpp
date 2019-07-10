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
#include <cstdlib>
#include <ctime>
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
#include "stat.hpp"
#include "grafana.hpp"

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
void coutRaw(uint8_t* buffer, size_t size);
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
    boost::asio::steady_timer timerSendTransferTransaction;
    boost::asio::steady_timer timerPerformanceTest;
    TestInfo(boost::asio::io_context& ioc, uint64_t period, uint32_t eachTime):
            timerSendTransferTransaction(ioc),
            timerPerformanceTest(ioc),
            batch(eachTime), timer_timeout(period) { };
    void update(const signed_block &msg) {
        this->head_block_id = msg.previous;
    }

};

class TimeCostGuard {
    string thing;
    fc::time_point start;
    ostream & out;
    TimeCostGuard() = delete;
public:
    TimeCostGuard(ostream& out, string thing):out(out),thing(thing) {
        start = fc::time_point::now();
        //out << "Begin do \"" << thing << "\"" << endl;
    }
    virtual ~TimeCostGuard() {
        auto cost = (fc::time_point::now() - start).count()/1000.0;
        if(cost > 1) { // > 1ms
            out << "End do \"" << thing << "\"," << "time cost " << cost << " ms" << endl;
        }
    }
};

struct OutQueue {
    std::list<Buffer> _q;
    uint8_t *pTemp= nullptr;
    std::size_t wIndex = 0;
    static constexpr std::size_t allocSize = 1024;
    AsyncBufferPool& pool;
    OutQueue() = delete;
    OutQueue(AsyncBufferPool& pool):pool(pool) {
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
    bool empty(void) {
        return _q.empty();
    }
    void pop_front(void) {
        _q.pop_front();
    }
};

struct StatData {
    uint64_t tsn{0};//总消息条数
    uint64_t sn[MsgType::MESSAGELEN];//各消息的条数
    uint64_t tBytesCount{0}; //总消息字节数
    uint64_t bytesCount[MsgType::MESSAGELEN]; //各个消息字节数
    fc::time_point last;
    static string MsgTypeStr[MsgType::MESSAGELEN];
    StatData() {
        for(auto i = 0 ; i < MsgType::MESSAGELEN; i++){
            sn[i] = 0;
            bytesCount[i] = 0;
        }
        last = fc::time_point::now();
    }
    void print(ostream& out){
        auto now = fc::time_point::now();
        if((now - last).count() >= 5000000) {
            last = now;
            out << "total all messages number:" << tsn
                << ", total all messages bytes number:" << tBytesCount << endl;

            auto pbft_counts = sn[MsgType::PBFT_PREPARE] + sn[MsgType::PBFT_COMMIT] + sn[MsgType::PBFT_VIEW_CHANGE]
                    + sn[MsgType::PBFT_NEW_VIEW] + sn[MsgType::PBFT_CHECKPOINT] + sn[MsgType::PBFT_STABLE_CHECKPOINT]
                    + sn[MsgType::CHECKPOINT_REQUEST] + sn[MsgType::COMPRESSED_PBFT];
            auto pbft_bytes = bytesCount[MsgType::PBFT_PREPARE] + bytesCount[MsgType::PBFT_COMMIT]
                    + bytesCount[MsgType::PBFT_VIEW_CHANGE] + bytesCount[MsgType::PBFT_NEW_VIEW]
                    + bytesCount[MsgType::PBFT_CHECKPOINT] + bytesCount[MsgType::PBFT_STABLE_CHECKPOINT]
                    + bytesCount[MsgType::CHECKPOINT_REQUEST] + bytesCount[MsgType::COMPRESSED_PBFT];
            if(pbft_counts == 0 or pbft_bytes == 0 or tsn == 0 or tBytesCount == 0) return;
            out << "message number, message bytes number, % of pbft message number, % of pbft message bytes number"
                << ", % of message number, % of message bytes number"<< endl;
            for(auto i = 0; i < MsgType::MESSAGELEN; i++) {
                out << MsgTypeStr[i] << ": " << sn[i] << ","
                                             << bytesCount[i] << ","
                                             << sn[i]/double(pbft_counts)*100 << ","
                                             << bytesCount[i]/double(pbft_bytes)*100 << ","
                                             << sn[i]/double(tsn)*100 << ","
                                             << bytesCount[i]/double(tBytesCount)*100 << endl;
            }
            out << "--------------------------------------------------------------------------------------------------------------------------------" << endl;
        }
    }
};

class Client : public std::enable_shared_from_this<Client>{
    uint32_t statSeconds; //统计时间间隔
    Grafana grafana;
    TestInfo testInfo;
    tcp::socket _socket;
    tcp::resolver _resolver;
    boost::asio::steady_timer timerMakePeerSync;
    boost::asio::steady_timer timerSendTimeMessage;
    boost::asio::steady_timer timerOutQueue;
    fc::message_buffer<1024*1024*20> _messageBuffer;
    fc::optional<std::size_t> _outStandingReadBytes; //下次需要读取的字节数
    AsyncBufferPool bufferPool;
    ostream &output = cout;
    string host;
    string port;
    OutQueue outQueue{bufferPool};
    boost::asio::io_context& ioc;
    StatData statData;
    bool testTps;
    Stat<pbft_prepare> stat{10};
    void makePeerSync(void);
    void performanceTest(void);
    void startGeneration(const string& salt);
    void sendTransferTransaction();
    void reConnect(void) {
        _resolver.async_resolve(tcp::v4(), host, port,
                std::bind(&Client::OnResolve, this,
                        std::placeholders::_1,
                        std::placeholders::_2));
    }
    void DoSendoutData(void);
    void StartSendoutData(void);
    void toGrafana(MsgType type, std::size_t len);
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
           const char* contractName,
           uint64_t period,
           uint32_t eachTime);
    Client(boost::asio::io_context& ioc, const char* host, const char* port, const char* grafanaIP, const char* grafanPort);
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
    void sendMessage(packed_transaction&& msg);

    void checkQueueStatus(void);

    template <typename T>
    void sendMessage(T && msg) {
        checkQueueStatus();
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
