//
// Created by zwg on 19-6-12.
//
#include "client.hpp"
#include "outbuffer.hpp"
enum { BUF_SIZE = 1024 };

class queued_buffer : boost::noncopyable {
public:
    void clear_write_queue() {
        _write_queue.clear();
        _sync_write_queue.clear();
        _write_queue_size = 0;
    }

    void clear_out_queue() {
        while ( _out_queue.size() > 0 ) {
            _out_queue.pop_front();
        }
    }

    uint32_t write_queue_size() const { return _write_queue_size; }

    bool is_out_queue_empty() const { return _out_queue.empty(); }

    bool ready_to_send() const {
        // if out_queue is not empty then async_write is in progress
        return ((!_sync_write_queue.empty() || !_write_queue.empty()) && _out_queue.empty());
    }

    //
    bool add_write_queue( const std::shared_ptr<vector<char>>& buff,
                          std::function<void( boost::system::error_code, std::size_t )> callback,
                          bool to_sync_queue ) {
        if( to_sync_queue ) {
            _sync_write_queue.push_back( {buff, callback} );
        } else {
            _write_queue.push_back( {buff, callback} );
        }
        _write_queue_size += buff->size();
        if( _write_queue_size > 2 * def_max_write_queue_size ) {
            return false;
        }
        return true;
    }

    // 将本类缓存的数据放入到bufs里面去
    // _sync_write_queue的优先级高于_write_queue
    void fill_out_buffer( std::vector<boost::asio::const_buffer>& bufs ) {
        if( _sync_write_queue.size() > 0 ) { // always send msgs from sync_write_queue first
            fill_out_buffer( bufs, _sync_write_queue );
        } else { // postpone real_time write_queue if sync queue is not empty
            fill_out_buffer( bufs, _write_queue );
           std::cerr << "write queue size expected to be zero" << std::endl;
        }
    }

    void out_callback( boost::system::error_code ec, std::size_t w ) {
        for( auto& m : _out_queue ) {
            m.callback( ec, w );
        }
    }

private:
    struct queued_write;
    void fill_out_buffer( std::vector<boost::asio::const_buffer>& bufs,
                          std::deque<queued_write>& w_queue ) {
        while ( w_queue.size() > 0 ) {
            auto& m = w_queue.front();
            bufs.push_back( boost::asio::buffer( *m.buff ));
            _write_queue_size -= m.buff->size();
            _out_queue.emplace_back( m );
            w_queue.pop_front();
        }
    }

private:
    struct queued_write {
        std::shared_ptr<vector<char>> buff;
        std::function<void( boost::system::error_code, std::size_t )> callback;
    };

    uint32_t _write_queue_size = 0;
    std::deque<queued_write> _write_queue;
    std::deque<queued_write> _sync_write_queue; // sync_write_queue will be sent first
    std::deque<queued_write> _out_queue;

}; // queued_buffer

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
        const char* contractName):
_socket(ioc), _resolver(ioc), _timer(ioc), testInfo(ioc), ioc(ioc){
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
    //buffer = new u_char[1024*1024*16];
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

bool Client::processNextMessage(uint32_t messageLen) {
    try {
        auto ds = _messageBuffer.create_datastream();
        net_message msg;
        fc::raw::unpack(ds, msg);
        MsgHandler msgHandler(this);
        if (msg.contains<signed_block>()) msgHandler(std::move(msg.get<signed_block>()));
        else if (msg.contains<packed_transaction>()) msgHandler(std::move(msg.get<packed_transaction>()));
        else msg.visit(msgHandler);
    } catch(const fc::exception& e) {
        return false;
    }
    return true;
}

void Client::handleMessage(const handshake_message& msg) {
    output << "handshake_message ----------";
    output << msg << endl;
}

void Client::handleMessage(const chain_size_message& msg) {
    output << "chain_size_message ----------";
    output << msg << endl;
}

void Client::handleMessage(const go_away_message& msg) {
    output << "go_away_message ----------";
    output << msg << endl;
}

void Client::handleMessage(const time_message& msg) {
    output << "time_message ----------";
    output << msg << endl;
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

eosio::chain::action create_action_delegatebw(const name &from, const name &to, const asset &net, const asset &cpu, const fc::microseconds &abi_serializer_max_time){
    fc::variant variant_delegate = fc::mutable_variant_object()
            ("from", from.to_string())
            ("receiver", to.to_string())
            ("stake_net_quantity", net.to_string())
            ("stake_cpu_quantity", cpu.to_string())
            ("transfer", true);
    auto first = fc::json::from_string(eosio_system_abi).as<abi_def>();
    abi_serializer eosio_system_serializer{first, abi_serializer_max_time};

    auto payload_delegate = eosio_system_serializer.variant_to_binary( "delegatebw", variant_delegate, abi_serializer_max_time);
    eosio::chain::action act_delegate{vector<chain::permission_level>{{from,"active"}},
                                      config::system_account_name, eosio::chain::string_to_name("delegatebw"), payload_delegate};
    return act_delegate;
}

eosio::chain::action create_action_buyram(const name &from, const name &to, const asset &quant, const fc::microseconds &abi_serializer_max_time){
    fc::variant variant_buyram = fc::mutable_variant_object()
            ("payer", from.to_string())
            ("receiver", to.to_string())
            ("quant", quant.to_string());
    abi_serializer eosio_system_serializer{fc::json::from_string(eosio_system_abi).as<abi_def>(), abi_serializer_max_time};

    auto payload_buyram = eosio_system_serializer.variant_to_binary( "buyram", variant_buyram, abi_serializer_max_time);
    eosio::chain::action act_buyram{vector<chain::permission_level>{{from,"active"}},
                                    config::system_account_name,
                                    eosio::chain::string_to_name("buyram"),
                                    payload_buyram};

    return act_buyram;
}


void Client::startGeneration(const string& salt, const uint64_t& period, const uint64_t& batch_size) {
    /*if(period < 1 || period > 100)
        throw fc::exception(fc::invalid_operation_exception_code);
    if(batch_size < 1 || batch_size > 1000000)
        throw fc::exception(fc::invalid_operation_exception_code);*/
    auto abi_serializer_max_time = testInfo.abi_serializer_max_time;
    abi_serializer eosio_token_serializer{fc::json::from_string(eosio_token_abi).as<abi_def>(), abi_serializer_max_time};
    //create the actions here
    testInfo.act_a_to_b.account = testInfo.contractName;
    testInfo.act_a_to_b.name = name("transfer");
    testInfo.act_a_to_b.authorization = vector<permission_level>{{testInfo.user1,config::active_name}};
    testInfo.act_a_to_b.data = eosio_token_serializer.variant_to_binary("transfer",
            fc::json::from_string(
                    fc::format_string("{\"from\":\"${user1}\",\"to\":\"${user2}\","
                                      "\"quantity\":\"1.0000 ${token}\",\"memo\":\"${l}\"}",
                    fc::mutable_variant_object()
                    ("l", salt)
                    ("user1", testInfo.user1.to_string())
                    ("user2", testInfo.user2.to_string())
                    ("token", testInfo.tokenName))),
            abi_serializer_max_time);

    testInfo.act_b_to_a.account = testInfo.contractName;
    testInfo.act_b_to_a.name = name("transfer");
    testInfo.act_b_to_a.authorization = vector<permission_level>{{testInfo.user2,config::active_name}};
    testInfo.act_b_to_a.data = eosio_token_serializer.variant_to_binary("transfer",
            fc::json::from_string(
                    fc::format_string("{\"from\":\"${user2}\",\"to\":\"${user1}\","
                                      "\"quantity\":\"1.0000 ${token}\",\"memo\":\"${l}\"}",
                    fc::mutable_variant_object()
                    ("l", salt)
                    ("user1", testInfo.user1.to_string())
                    ("user2", testInfo.user2.to_string())
                    ("token", testInfo.tokenName))),
            abi_serializer_max_time);

    testInfo.timer_timeout = period;
    testInfo.batch = batch_size;
    sendTransferTransaction();
}

void Client::sendTransferTransaction() {
    OutputGuard og(output, string("sendTransferTransaction"));
    try {
        auto chainid = testInfo.chain_id;
        static uint64_t nonce = static_cast<uint64_t>(fc::time_point::now().sec_since_epoch()) << 32;

        block_id_type reference_block_id = testInfo.head_block_id;

        for(unsigned int i = 0; i < testInfo.batch; ++i) {
            {
                signed_transaction trx;
                trx.actions.push_back(testInfo.act_a_to_b);
                trx.context_free_actions.emplace_back(action({}, config::null_account_name, "nonce", fc::raw::pack(nonce++)));
                trx.set_reference_block(reference_block_id);
                trx.expiration = fc::time_point::now() + fc::seconds(30);
                trx.max_net_usage_words = 100;
                trx.sign(testInfo.user1PK, chainid);
                sendMessage(packed_transaction(trx));
            }
            {
                signed_transaction trx;
                trx.actions.push_back(testInfo.act_b_to_a);
                trx.context_free_actions.emplace_back(action({}, config::null_account_name, "nonce", fc::raw::pack(nonce++)));
                trx.set_reference_block(reference_block_id);
                trx.expiration = fc::time_point::now() + fc::seconds(30);
                trx.max_net_usage_words = 100;
                trx.sign(testInfo.user2PK, chainid);
                sendMessage(packed_transaction(trx));
            }
        }
    } catch ( const fc::exception& e ) {
        output << "fc::exception :" << e.what() << endl;
    }
    testInfo._timer.expires_after(std::chrono::microseconds(testInfo.timer_timeout));
    testInfo._timer.async_wait(std::bind(&Client::sendTransferTransaction, this));
}



//void Client::createTestAccounts(const string& init_name, const string& init_priv_key, const string& core_symbol) {
//    OutputGuard og(output, string("createTestAccounts"));
//    name newaccountA("aaaaaaaaaaaa");
//    name newaccountB("bbbbbbbbbbbb");
//    name newaccountC("cccccccccccc");
//    name creator(init_name);
//
//    auto p = fc::json::from_string(eosio_token_abi);
//    eosio::chain::abi_def currency_abi_def = p.as<eosio::chain::abi_def>();
//    auto & abi_serializer_max_time = testInfo.abi_serializer_max_time;
//    eosio::chain::abi_serializer eosio_token_serializer{
//        fc::json::from_string(eosio_token_abi).as<eosio::chain::abi_def>(),
//        abi_serializer_max_time
//    };
//    //chain_id_type chainid("02ce5aa2a05e594cc3c041fc8989cf8ff4fd6efe828a23d7b3825d51b70fa2ed");
//    fc::crypto::private_key txn_test_receiver_A_priv_key = fc::crypto::private_key::regenerate(fc::sha256(std::string(64, 'a')));
//    fc::crypto::private_key txn_test_receiver_B_priv_key = fc::crypto::private_key::regenerate(fc::sha256(std::string(64, 'b')));
//    fc::crypto::private_key txn_test_receiver_C_priv_key = fc::crypto::private_key::regenerate(fc::sha256(std::string(64, 'c')));
//    fc::crypto::public_key  txn_text_receiver_A_pub_key = txn_test_receiver_A_priv_key.get_public_key();
//    fc::crypto::public_key  txn_text_receiver_B_pub_key = txn_test_receiver_B_priv_key.get_public_key();
//    fc::crypto::public_key  txn_text_receiver_C_pub_key = txn_test_receiver_C_priv_key.get_public_key();
//    fc::crypto::private_key creator_priv_key = fc::crypto::private_key(init_priv_key);
//
//    eosio::chain::asset net{1000000, symbol(4,core_symbol.c_str())};
//    eosio::chain::asset cpu{10000000000, symbol(4,core_symbol.c_str())};
//    eosio::chain::asset ram{1000000, symbol(4,core_symbol.c_str())};
//
//    {
//        signed_transaction trx;
//        //create "A" account
//        {
//            auto owner_auth   = eosio::chain::authority{1, {{txn_text_receiver_A_pub_key, 1}}, {}};
//            auto active_auth  = eosio::chain::authority{1, {{txn_text_receiver_A_pub_key, 1}}, {}};
//            trx.actions.emplace_back(vector<chain::permission_level>{{creator,"active"}}, newaccount{creator, newaccountA, owner_auth, active_auth});
//            //delegate cpu net and buyram
//            auto act_delegatebw = create_action_delegatebw(creator, newaccountA,net,cpu,abi_serializer_max_time);
//            auto act_buyram = create_action_buyram(creator, newaccountA, ram, abi_serializer_max_time);
//            trx.actions.emplace_back(act_delegatebw);
//            trx.actions.emplace_back(act_buyram);
//        }
//        //create "B" account
//        {
//            auto owner_auth   = eosio::chain::authority{1, {{txn_text_receiver_B_pub_key, 1}}, {}};
//            auto active_auth  = eosio::chain::authority{1, {{txn_text_receiver_B_pub_key, 1}}, {}};
//
//            trx.actions.emplace_back(vector<chain::permission_level>{{creator,"active"}}, newaccount{creator, newaccountB, owner_auth, active_auth});
//
//            //delegate cpu net and buyram
//            auto act_delegatebw = create_action_delegatebw(creator, newaccountB,net,cpu,abi_serializer_max_time);
//            auto act_buyram = create_action_buyram(creator, newaccountB, ram, abi_serializer_max_time);
//
//            trx.actions.emplace_back(act_delegatebw);
//            trx.actions.emplace_back(act_buyram);
//        }
//        //create "cccccccccccc" account
//        {
//            auto owner_auth   = eosio::chain::authority{1, {{txn_text_receiver_C_pub_key, 1}}, {}};
//            auto active_auth  = eosio::chain::authority{1, {{txn_text_receiver_C_pub_key, 1}}, {}};
//
//            trx.actions.emplace_back(vector<chain::permission_level>{{creator,"active"}}, newaccount{creator, newaccountC, owner_auth, active_auth});
//
//            //delegate cpu net and buyram
//            auto act_delegatebw = create_action_delegatebw(creator, newaccountC,net,cpu,abi_serializer_max_time);
//            auto act_buyram = create_action_buyram(creator, newaccountC, ram, abi_serializer_max_time);
//
//            trx.actions.emplace_back(act_delegatebw);
//            trx.actions.emplace_back(act_buyram);
//        }
//        //trx.expiration = testInfo.timestamp.to_time_point() + fc::seconds(30);
//        trx.expiration = fc::time_point::now() + fc::seconds(30);
//        trx.set_reference_block(testInfo.head_block_id);
//        trx.sign(creator_priv_key, testInfo.chain_id);
//        sendMessage(std::move(packed_transaction(trx)));
//    }
//
//    /*//set cccccccccccc contract to eosio.token & initialize it
//    {
//        signed_transaction trx;
//
//        vector<uint8_t> wasm = wast_to_wasm(std::string(eosio_token_wast));
//
//        setcode handler;
//        handler.account = newaccountC;
//        handler.code.assign(wasm.begin(), wasm.end());
//
//        trx.actions.emplace_back( vector<chain::permission_level>{{newaccountC,"active"}}, handler);
//
//        {
//            setabi handler;
//            handler.account = newaccountC;
//            handler.abi = fc::raw::pack(json::from_string(eosio_token_abi).as<abi_def>());
//            trx.actions.emplace_back( vector<chain::permission_level>{{newaccountC,"active"}}, handler);
//        }
//
//        {
//            action act;
//            act.account = N(cccccccccccc);
//            act.name = N(create);
//            act.authorization = vector<permission_level>{{newaccountC,config::active_name}};
//            act.data = eosio_token_serializer.variant_to_binary("create", fc::json::from_string("{\"issuer\":\"cccccccccccc\",\"maximum_supply\":\"1000000000.0000 CUR\"}}"), abi_serializer_max_time);
//            trx.actions.push_back(act);
//        }
//        {
//            action act;
//            act.account = N(cccccccccccc);
//            act.name = N(issue);
//            act.authorization = vector<permission_level>{{newaccountC,config::active_name}};
//            act.data = eosio_token_serializer.variant_to_binary("issue", fc::json::from_string("{\"to\":\"cccccccccccc\",\"quantity\":\"1000000000.0000 CUR\",\"memo\":\"\"}"), abi_serializer_max_time);
//            trx.actions.push_back(act);
//        }
//        {
//            action act;
//            act.account = N(cccccccccccc);
//            act.name = N(transfer);
//            act.authorization = vector<permission_level>{{newaccountC,config::active_name}};
//            act.data = eosio_token_serializer.variant_to_binary("transfer", fc::json::from_string("{\"from\":\"cccccccccccc\",\"to\":\"aaaaaaaaaaaa\",\"quantity\":\"500000000.0000 CUR\",\"memo\":\"\"}"), abi_serializer_max_time);
//            trx.actions.push_back(act);
//        }
//        {
//            action act;
//            act.account = N(cccccccccccc);
//            act.name = N(transfer);
//            act.authorization = vector<permission_level>{{newaccountC,config::active_name}};
//            act.data = eosio_token_serializer.variant_to_binary("transfer", fc::json::from_string("{\"from\":\"cccccccccccc\",\"to\":\"bbbbbbbbbbbb\",\"quantity\":\"500000000.0000 CUR\",\"memo\":\"\"}"), abi_serializer_max_time);
//            trx.actions.push_back(act);
//        }
//
//        trx.expiration = cc.head_block_time() + fc::seconds(30);
//        trx.set_reference_block(cc.head_block_id());
//        trx.max_net_usage_words = 5000;
//        trx.sign(txn_test_receiver_C_priv_key, chainid);
//        trxs.emplace_back(std::move(trx));
//    }*/
//
//
//
//}

void Client::executeTest(void) {
    startGeneration("abcdefg", 10, 10000);
}


void Client::performanceTest(void) {
    OutputGuard og(output, string("performanceTest"));
    //createTestAccounts("eosio", "5KQwrPbwdL6PhXujxW37FSSQZ1JiwsST4cqQzDeyXtP79zkvFD3", "BOS");
    executeTest();
}


void Client::handleMessage(const notice_message& msg) {

//    sync_request_message请求
//    if(msg.known_blocks.mode == last_irr_catch_up && msg.known_trx.mode == last_irr_catch_up) {
//        sync_request_message srm{1, 100};
//        sendSyncRequestMessage(std::move(srm));
//    }
    output << "notice_message ----------";
    output << msg << endl;

    // 让对端认为数据已经同步(将sync->true)
    _timer.expires_after(std::chrono::seconds(10));
    _timer.async_wait(std::bind(&Client::makePeerSync, this));

    // 开始性能测试
    _timer.expires_after(std::chrono::seconds(50));
    _timer.async_wait(std::bind(&Client::performanceTest, this));
}

void Client::handleMessage(const request_message& msg) {
    output << "request_message ----------";
    output << msg << endl;
}

void Client::handleMessage(const sync_request_message& msg) {
    output << "sync_request_message ----------";
    output << msg << endl;
}

void Client::handleMessage(const signed_block_ptr& msg) {
    output << "signed_block_message ----------";
    output << *msg << endl;
    testInfo.update(*msg);
}

void Client::handleMessage(const packed_transaction_ptr& msg) {
    output << "packed_transaction_ptr ----------";
    output << *msg << endl;
}

void Client::handleMessage(const response_p2p_message& msg) {
    output << "response_p2p_message ----------" << endl;
}

void Client::handleMessage(const request_p2p_message& msg) {
    output << "request_p2p_message ----------" << endl;
}

void Client::handleMessage(const pbft_prepare &msg) {
    //output << "pbft_prepare ----------";
    //output << msg << endl;
}

void Client::handleMessage(const pbft_commit &msg) {
    //output << "pbft_commit ----------";
    //output << msg << endl;
}

void Client::handleMessage(const pbft_view_change &msg) {
    output << "pbft_view_change ----------";
    output << msg << endl;
}

void Client::handleMessage(const pbft_new_view &msg) {
    output << "pbft_new_view ----------";
    output << msg << endl;
}

void Client::handleMessage(const pbft_checkpoint &msg) {
    output << "pbft_checkpoint ----------";
    output << msg << endl;
}

void Client::handleMessage(const pbft_stable_checkpoint &msg) {
    output << "pbft_stable_checkpoint ----------";
    output << msg << endl;
}

void Client::handleMessage(const checkpoint_request_message &msg) {
    output << "checkpoint_request_message ----------";
    output << msg << endl;
}

void Client::handleMessage(const compressed_pbft_message &msg) {
    output << "compressed_pbft_message ----------" << endl;
}


void Client::StartReadMessage() {
    if(!_socket.is_open())
        return;
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
                                if(!processNextMessage(messageLength)) return;
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

void Client::DoSendTimeMessage() {
    if(!_socket.is_open())
        return;
    auto time = std::chrono::system_clock::now().time_since_epoch().count();
    time_message msg;
    msg.rec = time;
    msg.org = 0;
    msg.xmt = time;

    auto n = net_message(msg);

    uint32_t payload_size = fc::raw::pack_size(n);
    char *header = reinterpret_cast<char *>(&payload_size);
    size_t header_size = sizeof(payload_size);
    size_t buffer_size = header_size + payload_size;

    uint8_t * buffer = bufferPool.newBuffer(buffer_size);
    fc::datastream<uint8_t *> ds(buffer, buffer_size);
    ds.write(header, header_size);
    fc::raw::pack(ds, n);

    _socket.async_write_some(
            boost::asio::buffer(buffer, buffer_size),
            [this, buffer](boost::system::error_code ec, std::size_t w){
                bufferPool.deleteBuffer(buffer);
                if(ec) {
                    cerr << "Error when asyn_write_some." << endl;
                    cerr << ec.message() << endl;
                    _socket.close();
                    ioc.stop();
                    return;
                }
                output << "async write successfully." << endl;
            });
    _timer.expires_after(std::chrono::seconds(50));
    _timer.async_wait(std::bind(&Client::DoSendTimeMessage, this));
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


int main(int argc, char* argv[]) {
    cout << argc << endl;
    for(auto i = 1; i < argc; i++) {
        cout << argv[i] << endl;
    }
    if(argc != 10) {
        cerr << "Invalid parameter." << endl;
        cerr << "Usage: ./client ip port chain_id user1 private_key_of_user1 user2 private_key_of_user2 token_name contract_name" << endl;
        cerr << "Example: ./client 127.0.0.1 9876 "
             << "cf057bbfb72640471fd910bcb67639c22df9f92470936cddc1ade0e2f2e7dc4f "
             << "aaaaaaaaaaaa 5HsvPQ2wBttkMMYXfJUw2QW5pYh5ReSqVBqPhprWh3GGhiQyezC "
             << "bbbbbbbbbbbb 5JAghZg5An1L8DdT75CyQSaZAHuofY9mst52oCW9gQUQjs1n76L "
             << "BOS eosio.token" << endl;
        exit(1);
    }
    const char* host = argv[1];
    const char* port = argv[2];
    const char* chain_id = argv[3];
    const char* user1 = argv[4];
    const char* user1PK = argv[5];
    const char* user2 = argv[6];
    const char* user2PK = argv[7];
    const char* tokenName = argv[8];
    const char* contractName = argv[9];

    while(true) {
        boost::asio::io_service ioc;
        Client client(ioc, host, port, chain_id, user1, user1PK, user2, user2PK, tokenName, contractName);
        ioc.run();
        cout << "Reconnect." << endl;
    }
}