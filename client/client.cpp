//
// Created by zwg on 19-6-12.
//
#include "client.hpp"
#include "outbuffer.hpp"
#include "stat.hpp"

MsgHandler::MsgHandler(Client* p):_pCli(p) { }

Client::Client(
        boost::asio::io_context& ioc,
        const char* host,
        const char* port,
        const char* cid,
        const char* user1,
        const char* user1PK,
        const char* user2,
        const char* user2PK,
        const char* tokenName,
        const char* contractName,
        uint64_t period,
        uint32_t eachTime):
_socket(ioc), _resolver(ioc), ioc(ioc),grafana(ioc),
timerOutQueue(ioc), timerMakePeerSync(ioc),
timerSendTimeMessage(ioc), testInfo(ioc, period, eachTime) {
    this->host = string(host);
    this->port = string(port);
    reConnect();
    testInfo.chain_id = chain_id_type(string(cid));
    testInfo.user1 = name(string(user1));
    testInfo.user2 = name(string(user2));
    testInfo.user1PK = fc::crypto::private_key(string(user1PK));
    testInfo.user2PK = fc::crypto::private_key(string(user2PK));
    testInfo.tokenName = string(tokenName);
    testInfo.contractName = name(string(contractName));
    std::srand(std::time(nullptr));
    this->testTps = true;
    //buffer = new u_char[1024*1024*16];
}

Client::Client(boost::asio::io_context& ioc, const char* host, const char* port, uint32_t seconds):
ioc(ioc),_socket(ioc), _resolver(ioc), timerOutQueue(ioc), timerMakePeerSync(ioc),grafana(ioc),
timerSendTimeMessage(ioc), testInfo(ioc, 0, 0) {
    this->host = string(host);
    this->port = string(port);
    this->testTps = false;
    reConnect();
    std::srand(std::time(nullptr));
}

Client::~Client(void) {
    cout << "Client::~Client(void)" << endl;
    //delete [] buffer;
}

void Client::OnConnect(boost::system::error_code ec, tcp::endpoint endpoint) {
    if (ec) {
        std::cout << "Connect failed: " << ec.message() << std::endl;
        _socket.close();
        ioc.stop();
        return;
    }
    StartSendTimeMessage();
    StartReadMessage();
    StartHandshakeMessage();
    StartSendoutData();
}

void Client::sendHandshakeMessage(handshake_message && msg) {
   sendMessage(msg);
}

void Client::StartHandshakeMessage(void) {
    auto msg = FakeData::fakeHandShakeMessage(testInfo.chain_id);
    sendHandshakeMessage(std::move(msg));

    /* 此数据发送会导致端口重置，发送出去会返回goaway message*/
//    msg = FakeData::invalidFakeHandShakeMessage();
//    sendHandshakeMessage(std::move(msg));

//    msg = FakeData::acceptHandshakeMessage();
//    sendHandshakeMessage(std::move(msg));
}

void Client::toGrafana(MsgType type, std::size_t len) {
    if(testTps) return;
    TimeCostGuard tcg(output, "Client::toGrafana");
    stringstream ss;
    ss << "msgstat,m=" << type
       << " len=" << len << " " << fc::time_point::now().time_since_epoch().count()*1000;
    auto p = ss.str();
    grafana.send(p.c_str(), p.length());
}


bool Client::processNextMessage(uint32_t messageLen) {
    try {
        auto ds = _messageBuffer.create_datastream();
        net_message msg;
        fc::raw::unpack(ds, msg);
        MsgHandler msgHandler(this);
        if (msg.contains<signed_block>()) msgHandler(std::move(msg.get<signed_block>()));
        else if (msg.contains<packed_transaction>()) msgHandler(std::move(msg.get<packed_transaction>()));
        else msg.visit(msgHandler);
        if(!testTps) {
            statData.tsn++;
            statData.tBytesCount += messageLen;
            if (msg.contains<handshake_message>()) {
                statData.sn[MsgType::HANDSHAKE]++;
                statData.bytesCount[MsgType::HANDSHAKE] += messageLen;
                toGrafana(MsgType::HANDSHAKE, messageLen);
            }
            else if (msg.contains<chain_size_message>()) {
                statData.sn[MsgType::CHAIN_SIZE]++;
                statData.bytesCount[MsgType::CHAIN_SIZE] += messageLen;
                toGrafana(MsgType::CHAIN_SIZE, messageLen);
            }
            else if (msg.contains<go_away_message>()) {
                statData.sn[MsgType::GO_AWAY]++;
                statData.bytesCount[MsgType::GO_AWAY] += messageLen;
                toGrafana(MsgType::GO_AWAY, messageLen);
            }
            else if (msg.contains<time_message>()) {
                statData.sn[MsgType::TIME]++;
                statData.bytesCount[MsgType::TIME] += messageLen;
                toGrafana(MsgType::TIME, messageLen);
            }
            else if (msg.contains<notice_message>()) {
                statData.sn[MsgType::NOTICE]++;
                statData.bytesCount[MsgType::NOTICE] += messageLen;
                toGrafana(MsgType::NOTICE, messageLen);
            }
            else if (msg.contains<request_message>()) {
                statData.sn[MsgType::REQUEST]++;
                statData.bytesCount[MsgType::REQUEST] += messageLen;
                toGrafana(MsgType::REQUEST, messageLen);
            }
            else if (msg.contains<sync_request_message>()) {
                statData.sn[MsgType::SYNC_REQUEST]++;
                statData.bytesCount[MsgType::SYNC_REQUEST] += messageLen;
                toGrafana(MsgType::SYNC_REQUEST, messageLen);
            }
            else if (msg.contains<signed_block>()) {
                statData.sn[MsgType::SIGNED_BLOCK]++;
                statData.bytesCount[MsgType::SIGNED_BLOCK] += messageLen;
                toGrafana(MsgType::SIGNED_BLOCK, messageLen);
            }
            else if (msg.contains<packed_transaction>()) {
                statData.sn[MsgType::PACKED_TRANSACTION]++;
                statData.bytesCount[MsgType::PACKED_TRANSACTION] += messageLen;
                toGrafana(MsgType::PACKED_TRANSACTION, messageLen);
            }
            else if (msg.contains<response_p2p_message>()) {
                statData.sn[MsgType::RESPONSE_P2P]++;
                statData.bytesCount[MsgType::RESPONSE_P2P] += messageLen;
                toGrafana(MsgType::CHAIN_SIZE, messageLen);
            }
            else if (msg.contains<request_p2p_message>()) {
                statData.sn[MsgType::REQUEST_P2P]++;
                statData.bytesCount[MsgType::REQUEST_P2P] += messageLen;
                toGrafana(MsgType::REQUEST_P2P, messageLen);
            }
            else if (msg.contains<pbft_prepare>()) {
                statData.sn[MsgType::PBFT_PREPARE]++;
                statData.bytesCount[MsgType::PBFT_PREPARE] += messageLen;
                toGrafana(MsgType::PBFT_PREPARE, messageLen);
            }
            else if (msg.contains<pbft_commit>()) {
                statData.sn[MsgType::PBFT_COMMIT]++;
                statData.bytesCount[MsgType::PBFT_COMMIT] += messageLen;
                toGrafana(MsgType::PBFT_COMMIT, messageLen);
            }
            else if (msg.contains<pbft_view_change>()) {
                statData.sn[MsgType::PBFT_VIEW_CHANGE]++;
                statData.bytesCount[MsgType::PBFT_VIEW_CHANGE] += messageLen;
                toGrafana(MsgType::PBFT_VIEW_CHANGE, messageLen);
            }
            else if (msg.contains<pbft_new_view>()) {
                statData.sn[MsgType::PBFT_NEW_VIEW]++;
                statData.bytesCount[MsgType::PBFT_NEW_VIEW] += messageLen;
                toGrafana(MsgType::PBFT_NEW_VIEW, messageLen);
            }
            else if (msg.contains<pbft_checkpoint>()) {
                statData.sn[MsgType::PBFT_CHECKPOINT]++;
                statData.bytesCount[MsgType::PBFT_CHECKPOINT] += messageLen;
                toGrafana(MsgType::PBFT_CHECKPOINT, messageLen);
            }
            else if (msg.contains<pbft_stable_checkpoint>()) {
                statData.sn[MsgType::PBFT_STABLE_CHECKPOINT]++;
                statData.bytesCount[MsgType::PBFT_STABLE_CHECKPOINT] += messageLen;
                toGrafana(MsgType::PBFT_STABLE_CHECKPOINT, messageLen);
            }
            else if (msg.contains<checkpoint_request_message>()) {
                statData.sn[MsgType::CHECKPOINT_REQUEST]++;
                statData.bytesCount[MsgType::CHECKPOINT_REQUEST] += messageLen;
                toGrafana(MsgType::CHECKPOINT_REQUEST, messageLen);
            }
            else if (msg.contains<compressed_pbft_message>()) {
                statData.sn[MsgType::COMPRESSED_PBFT]++;
                statData.bytesCount[MsgType::COMPRESSED_PBFT] += messageLen;
                toGrafana(MsgType::COMPRESSED_PBFT, messageLen);
            }
            else {
                cerr << "Invalid message type" << endl;
                return false;
            }
        }
        //statData.print(output);
    } catch(const fc::exception& e) {
        cerr << "fc::exception :" << e.what() << endl;
        return false;
    }
    return true;
}

void Client::handleMessage(const handshake_message& msg) {
//    output << "handshake_message ----------";
//    output << msg << endl;
}

void Client::handleMessage(const chain_size_message& msg) {
//    output << "chain_size_message ----------";
//    output << msg << endl;
}

void Client::handleMessage(const go_away_message& msg) {
//    output << "go_away_message ----------";
//    output << msg << endl;
}

void Client::handleMessage(const time_message& msg) {
//    output << "time_message ----------";
//    output << msg << endl;
}

// 让对端认为数据已经同步，向此节点传播数据
void Client::makePeerSync(void) {
    request_message rm;
    rm.req_blocks.mode = none;
    rm.req_blocks.pending = 0;
    rm.req_trx.mode = none;
    rm.req_trx.pending = 0;
    sendMessage(std::move(rm));
    output << "send request message successfully." << endl;
}

void Client::checkQueueStatus(void) {
    if(outQueue.size() >= 400) {
        cerr << "Warning:" <<"outQueue.size(" << outQueue.size() << ") > 400" << endl;
    }
    if(outQueue.size() >= 800) {
        //防御，防止内存沾满
        cerr << "Error:" <<"outQueue.size(" << outQueue.size() << ") > 800" << endl;
        _socket.close();
        ioc.stop();
        EOS_THROW(eosio::chain::queue_over_max_size_exception, "queue_over_max_size_exception.");
    }
}


void Client::sendMessage(packed_transaction&& msg) {
    checkQueueStatus();
    net_message netMsg(msg);
    int32_t payload_size = fc::raw::pack_size(netMsg);
    char* header = reinterpret_cast<char*>(&payload_size);
    size_t header_size = sizeof(payload_size);
    size_t messageLen = header_size + payload_size;
    if(messageLen > outQueue.allocSize - outQueue.wIndex) {
        outQueue.push_back(Buffer(outQueue.pTemp, outQueue.wIndex));
        outQueue.pTemp = bufferPool.newBuffer(outQueue.allocSize);
        outQueue.wIndex = 0;
    }

    fc::datastream<uint8_t*> ds(outQueue.pTemp + outQueue.wIndex, messageLen);
    ds.write(header, header_size);
    fc::raw::pack(ds, netMsg);
    outQueue.wIndex += messageLen;
}

void Client::startGeneration(const string& salt) {
    abi_serializer eosio_token_serializer{
        fc::json::from_string(eosio_token_abi).as<abi_def>(),
        testInfo.abi_serializer_max_time
    };
    //create the actions here
    testInfo.act_a_to_b.account = testInfo.contractName;
    testInfo.act_a_to_b.name = name("transfer");
    testInfo.act_a_to_b.authorization = vector<permission_level>{{testInfo.user1,config::active_name}};
    testInfo.act_a_to_b.data = eosio_token_serializer.variant_to_binary("transfer",
            fc::json::from_string(
                    fc::format_string("{\"from\":\"${user1}\",\"to\":\"${user2}\","
                                      "\"quantity\":\"0.0001 ${token}\",\"memo\":\"${l}\"}",
                    fc::mutable_variant_object()
                    ("l", salt)
                    ("user1", testInfo.user1.to_string())
                    ("user2", testInfo.user2.to_string())
                    ("token", testInfo.tokenName))),
            testInfo.abi_serializer_max_time);

    testInfo.act_b_to_a.account = testInfo.contractName;
    testInfo.act_b_to_a.name = name("transfer");
    testInfo.act_b_to_a.authorization = vector<permission_level>{{testInfo.user2,config::active_name}};
    testInfo.act_b_to_a.data = eosio_token_serializer.variant_to_binary("transfer",
            fc::json::from_string(
                    fc::format_string("{\"from\":\"${user2}\",\"to\":\"${user1}\","
                                      "\"quantity\":\"0.0001 ${token}\",\"memo\":\"${l}\"}",
                    fc::mutable_variant_object()
                    ("l", salt)
                    ("user1", testInfo.user1.to_string())
                    ("user2", testInfo.user2.to_string())
                    ("token", testInfo.tokenName))),
            testInfo.abi_serializer_max_time);

    sendTransferTransaction();
}

void Client::sendTransferTransaction() {
    try {
        testInfo.timerSendTransferTransaction.expires_after(std::chrono::microseconds(testInfo.timer_timeout));
        testInfo.timerSendTransferTransaction.async_wait(std::bind(&Client::sendTransferTransaction, this));
        auto chainid = testInfo.chain_id;
        static uint64_t nonce = static_cast<uint64_t>(fc::time_point::now().sec_since_epoch()) << 32;
        block_id_type reference_block_id = testInfo.head_block_id;
        for (auto i = 0; i<testInfo.batch; i++) {
            if (std::rand()%2==1) {
                signed_transaction trx;
                trx.actions.push_back(testInfo.act_a_to_b);
                trx.context_free_actions.emplace_back(
                        action({}, config::null_account_name, "nonce", fc::raw::pack(nonce++)));
                trx.set_reference_block(reference_block_id);
                trx.expiration = fc::time_point::now()+fc::seconds(30);
                trx.max_net_usage_words = 100;
                trx.sign(testInfo.user1PK, chainid);
                sendMessage(packed_transaction(trx));
            }
            else {
                signed_transaction trx;
                trx.actions.push_back(testInfo.act_b_to_a);
                trx.context_free_actions.emplace_back(
                        action({}, config::null_account_name, "nonce", fc::raw::pack(nonce++)));
                trx.set_reference_block(reference_block_id);
                trx.expiration = fc::time_point::now()+fc::seconds(30);
                trx.max_net_usage_words = 100;
                trx.sign(testInfo.user2PK, chainid);
                sendMessage(packed_transaction(trx));
            }
        }
    }catch (fc::exception e) {
        cerr << "fc::exception: " << e.what() << endl;
        testInfo.timerSendTransferTransaction.cancel();
    }catch (std::exception e) {
        cerr << "std::exception: " << e.what() << endl;
        testInfo.timerSendTransferTransaction.cancel();
    }
}

void Client::performanceTest(void) {
    startGeneration("abcdefg");
}


void Client::handleMessage(const notice_message& msg) {

//    output << "notice_message ----------";
//    output << msg << endl;

    // 让对端认为数据已经同步(将sync->true)
    timerMakePeerSync.expires_after(std::chrono::milliseconds(500));
    timerMakePeerSync.async_wait(std::bind(&Client::makePeerSync, this));

    // 开始性能测试
    if(this->testTps) {
        testInfo.timerPerformanceTest.expires_after(std::chrono::seconds(1));
        testInfo.timerPerformanceTest.async_wait(std::bind(&Client::performanceTest, this));
    }
}

void Client::handleMessage(const request_message& msg) {
//    output << "request_message ----------";
//    output << msg << endl;
}

void Client::handleMessage(const sync_request_message& msg) {
//    output << "sync_request_message ----------";
//    output << msg << endl;
}

void Client::handleMessage(const signed_block_ptr& msg) {
/*    output << "signed_block_message ----------";
    output << *msg << endl;*/
    testInfo.update(*msg);
}

void Client::handleMessage(const packed_transaction_ptr& msg) {
/*    output << "packed_transaction_ptr ----------";
    output << *msg << endl;*/
}

void Client::handleMessage(const response_p2p_message& msg) {
    //output << "response_p2p_message ----------" << endl;
}

void Client::handleMessage(const request_p2p_message& msg) {
    //output << "request_p2p_message ----------" << endl;
}

void Client::handleMessage(const pbft_prepare &msg) {
    //static uint64_t sn = 0;
    //stat.put(StatInfo<pbft_prepare>(msg, fc::time_point::now(), sn++));
    //stat.statInfo();
}

void Client::handleMessage(const pbft_commit &msg) {
/*    output << "pbft_commit ----------";
    output << msg << endl;*/
}

void Client::handleMessage(const pbft_view_change &msg) {
//    output << "pbft_view_change ----------";
//    output << msg << endl;
}

void Client::handleMessage(const pbft_new_view &msg) {
//    output << "pbft_new_view ----------";
//    output << msg << endl;
}

void Client::handleMessage(const pbft_checkpoint &msg) {
/*    output << "pbft_checkpoint ----------";
    output << msg << endl;*/
}

void Client::handleMessage(const pbft_stable_checkpoint &msg) {
//    output << "pbft_stable_checkpoint ----------";
//    output << msg << endl;
}

void Client::handleMessage(const checkpoint_request_message &msg) {
//    output << "checkpoint_request_message ----------";
//    output << msg << endl;
}

void Client::handleMessage(const compressed_pbft_message &msg) {
//    output << "compressed_pbft_message ----------" << endl;
}


void Client::StartReadMessage() {
    if(!_socket.is_open()) {
        return;
    }
    std::size_t minimum_read = _outStandingReadBytes ? *_outStandingReadBytes:message_header_size;
    auto completion_handler = [minimum_read](boost::system::error_code ec, std::size_t bytes_transferred) -> std::size_t {
        if(ec||bytes_transferred >= minimum_read) {
            return 0;
        }
        return minimum_read - bytes_transferred;
    };
    boost::asio::async_read(
            _socket,
            _messageBuffer.get_buffer_sequence_for_boost_async_read(),
            completion_handler,
            [this](boost::system::error_code ec, std::size_t bytes_transferred) {
                _outStandingReadBytes.reset();
                try{
                    if(!ec) {
                        //指针移动到下次写数据的位置
                        _messageBuffer.advance_write_ptr(bytes_transferred);
                        while(_messageBuffer.bytes_to_read() > 0) {
                            auto bytesInBuffer = _messageBuffer.bytes_to_read();
                            if(bytesInBuffer < message_header_size) {
                                //当前缓冲区的字节数不足4个字节(header_size存储数)
                                //接下来去socket上读取这么多字节，刚好将header读取到
                                _outStandingReadBytes.emplace(message_header_size - bytesInBuffer);
                                break;
                            }
                            // 缓冲区足够四个字节
                            uint32_t messageLength;
                            auto index = _messageBuffer.read_index();
                            _messageBuffer.peek(&messageLength, sizeof(messageLength), index);
                            // 异常场景
                            if(messageLength > def_send_buffer_size*2 || messageLength == 0) {
                                cerr << "Unexpected length of this message." << endl;
                                _socket.close();
                                ioc.stop();
                                return;
                            }
                            auto totalMessageLength = messageLength + message_header_size;
                            //当前缓存区已经有本条完整的数据
                            if(bytesInBuffer >= totalMessageLength) {
                                _messageBuffer.advance_read_ptr(message_header_size);
                                if(!processNextMessage(messageLength)) {
                                    _socket.close();
                                    ioc.stop();
                                    return;
                                }
                            } else { //当前缓存区没有完整的数据
                                auto outstandingMessageBytes = totalMessageLength - bytesInBuffer;
                                auto availableBufferBytes = _messageBuffer.bytes_to_write();
                                if(availableBufferBytes < outstandingMessageBytes)
                                    _messageBuffer.add_space(outstandingMessageBytes - availableBufferBytes);
                                _outStandingReadBytes.emplace(outstandingMessageBytes);
                                break;
                            }
                        }
                        StartReadMessage();
                    } else {
                        cerr << "error in read, " << ec.message() << endl;
                        _socket.close();
                        ioc.stop();
                    }
                } catch (...) {
                    cerr << "Catch exception." << endl;
                    _socket.close();
                    ioc.stop();
                }
            });
}

void Client::StartSendTimeMessage() {
    DoSendTimeMessage();
}

void Client::DoSendoutData(void) {
    static uint64_t sendCount = 0;
    sendCount++;
    try {
        if (!_socket.is_open()) {
            cerr << "!_socket.is_open().:" <<__func__ << endl;
            return;
        }

        if (outQueue.empty()) {
            auto sleep = testTps ? 20:20000;
            timerOutQueue.expires_after(std::chrono::microseconds(sleep));
            timerOutQueue.async_wait(std::bind(&Client::DoSendoutData, this));
            return;
        }
        if (sendCount%2000 == 0) output << "QueueLen:" << outQueue.size() << endl;
        auto& buff = outQueue.front();
        auto buffer = buff.pbuffer;
        auto bufferSize = buff.size;
        auto complete_handler = [bufferSize](boost::system::error_code ec,
                std::size_t bytes_transferred) -> std::size_t {
            return (ec || bufferSize<=bytes_transferred) ? 0 : (bufferSize-bytes_transferred);
        };

        auto begin = fc::time_point::now();
        boost::asio::async_write(
                _socket,
                boost::asio::buffer(buffer, bufferSize),
                complete_handler,
                [this, begin](boost::system::error_code ec, std::size_t w) {
                    auto timeCost = (fc::time_point::now() - begin).count();
                    if(timeCost > 1000) cerr << "async_write cost time:" << timeCost << endl;
                    auto& buff = this->outQueue.front();
                    auto bufferSize = buff.size;
                    this->bufferPool.deleteBuffer(buff.pbuffer);
                    this->outQueue.pop_front();

                    if (w!=buff.size)
                        cerr << "w != bufferSize:" << "w(" << w << "), (" << bufferSize << ")" << endl;
                    if (ec) {
                        cerr << "Error when asyn_write buffer." << endl;
                        cerr << ec.message() << endl;
                    }

                    if (ec || w!=buff.size) {
                        _socket.close();
                        ioc.stop();
                        return;
                    }

                    this->DoSendoutData();
                });
    }catch (fc::exception& e){
        cerr << "fc::exception :" << e.what() << endl;
    }catch (std::exception& e) {
        cerr << "std::exception :" << e.what() << endl;
    }
}

void Client::StartSendoutData(void) {
    DoSendoutData();
}

void Client::DoSendTimeMessage() {
    if(!_socket.is_open()){
        cerr << "check socket not open in " <<__func__<< endl;
        ioc.stop();
        return;
    }
    auto time = std::chrono::system_clock::now().time_since_epoch().count();
    time_message msg;
    msg.rec = time;
    msg.org = 0;
    msg.xmt = time;

    sendMessage(msg);
    timerSendTimeMessage.expires_after(std::chrono::seconds(5));
    timerSendTimeMessage.async_wait(std::bind(&Client::DoSendTimeMessage, this));
}

void Client::OnResolve(boost::system::error_code ec, tcp::resolver::results_type endpoints) {
    if(ec) {
        cerr << "Resolve error." << endl;
    } else {
        output << "Resolve ok." << endl;
        auto handle = std::bind(&Client::OnConnect, this, std::placeholders::_1, std::placeholders::_2);
        boost::asio::async_connect(_socket, endpoints, handle);
    }
}

string StatData::MsgTypeStr[MsgType::MESSAGELEN] = {
        string("handshake_message"), string("chain_size_message"), string("go_away_message"), string("time_message"),
        string("notice_message"), string("request_message"), string("sync_request_message"), string("signed_block"),
        string("packed_transaction"),  string("response_p2p_message"), string("request_p2p_message"), string("pbft_prepare"),
        string("pbft_commit"), string("pbft_view_change"), string("pbft_new_view"), string("pbft_checkpoint"),
        string("pbft_stable_checkpoint"), string("checkpoint_request_message"), string("compressed_pbft_message")
};

void testTps(char* argv[]) {
    const char* host = argv[2];
    const char* port = argv[3];
    const char* chain_id = argv[4];
    const char* user1 = argv[5];
    const char* user1PK = argv[6];
    const char* user2 = argv[7];
    const char* user2PK = argv[8];
    const char* tokenName = argv[9];
    const char* contractName = argv[10];
    uint64_t period = static_cast<uint64_t>(atol(argv[11]));
    uint32_t eachTime = static_cast<uint32_t>(atoi(argv[12]));
    while(true) {
        boost::asio::io_service ioc;
        Client client(
                ioc, host, port, chain_id,
                user1, user1PK, user2, user2PK,
                tokenName, contractName, period, eachTime);
        ioc.run();
        cout << "Reconnect." << endl;
    }
}

void stat(char* argv[]) {
    const char* host = argv[2];
    const char* port = argv[3];
    uint32_t seconds = static_cast<uint32_t>(atoi(argv[4]));
    while(true) {
        boost::asio::io_service ioc;
        Client client(ioc, host, port, seconds);
        ioc.run();
        cout << "Reconnect." << endl;
    }
}

int main(int argc, char* argv[]) {
    cout << argc << endl;
    for(auto i = 1; i < argc; i++) {
        cout << argv[i] << endl;
    }
    if(argc != 13 and argc != 5) {
        cerr << "Invalid parameter." << endl;
        cerr << "Usage: ./nodeos-tps test-tps ip port chain_id user1 private_key_of_user1 user2 private_key_of_user2 token_name contract_name microseconds_interval count_for_each" << endl;
        cerr << "Example: ./nodeos-tps test-tps 127.0.0.1 9876 "
             << "cf057bbfb72640471fd910bcb67639c22df9f92470936cddc1ade0e2f2e7dc4f "
             << "aaaaaaaaaaaa 5HsvPQ2wBttkMMYXfJUw2QW5pYh5ReSqVBqPhprWh3GGhiQyezC "
             << "bbbbbbbbbbbb 5JAghZg5An1L8DdT75CyQSaZAHuofY9mst52oCW9gQUQjs1n76L "
             << "BOS eosio.token 1000 2" << endl;
        cerr << "Usage: ./nodeos-tps stat ip port seconds" << endl;
        cerr << "Example: ./nodeos-tps stat 127.0.0.1 9876 10" << endl;
        exit(1);
    }

    const char* feature = argv[1];
    if(strcmp(feature, "test-tps") == 0) testTps(argv);
    else if (strcmp(feature, "stat") == 0) stat(argv);
    else cerr << "Invalid parameter." << endl;
    return 1;
}