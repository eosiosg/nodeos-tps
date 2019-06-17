//
// Created by zwg on 19-6-12.
//
#include "client.hpp"
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

Client::Client(boost::asio::io_context& ioc, const char* host, const char* port):
_socket(ioc), _resolver(ioc), _timer(ioc){
    _resolver.async_resolve(tcp::v4(), host, port,
                            std::bind(&Client::OnResolve, this,
                                      std::placeholders::_1,
                                      std::placeholders::_2));
    buffer = new u_char[1024*1024*16];
}

void Client::OnConnect(boost::system::error_code ec, tcp::endpoint endpoint) {
    if (ec) {
        std::cout << "Connect failed: " << ec.message() << std::endl;
        _socket.close();
        return;
    }
    StartSendTimeMessage();
    StartReadMessage();
    StartHandshakeMessage();
}

void Client::sendHandshakeMessage(handshake_message && msg) {
    auto n = net_message(msg);

    uint32_t payload_size = fc::raw::pack_size(n);
    char *header = reinterpret_cast<char *>(&payload_size);
    size_t header_size = sizeof(payload_size);
    size_t buffer_size = header_size + payload_size;


    fc::datastream<u_char *> ds(buffer, buffer_size);
    ds.write(header, header_size);
    fc::raw::pack(ds, n);

    _socket.async_write_some(
            boost::asio::buffer(buffer, buffer_size),
            [this](boost::system::error_code ec, std::size_t w){
                if(ec) {
                    cerr << "Error when asyn_write_some handshake_message." << endl;
                    cerr << ec.message() << endl;
                    _socket.close();
                    return;
                }
                cout << "async write handshake_message successfully." << endl;
            });
}

void Client::StartHandshakeMessage(void) {
    auto msg = FakeData::fakeHandShakeMessage();
    sendHandshakeMessage(std::move(msg));

    /* 此数据发送会导致端口重置，发送出去会返回goaway message
    msg = FakeData::invalidFakeHandShakeMessage();
    sendHandshakeMessage(std::move(msg));*/

    msg = FakeData::acceptHandshakeMessage();
    sendHandshakeMessage(std::move(msg));
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
    cout << "Client::handleMessage(const handshake_message& msg)" << endl;
}

void Client::handleMessage(const chain_size_message& msg) {
    cout << "Client::handleMessage(const chain_size_message& msg)" << endl;
}

void Client::handleMessage(const go_away_message& msg) {
    cout << "Client::handleMessage(const go_away_message& msg)" << endl;
}

void Client::handleMessage(const time_message& msg) {
    cout << "Client::handleMessage(const time_message& msg)" << endl;
}

void Client::handleMessage(const notice_message& msg) {
    cout << "Client::handleMessage(const notice_message& msg)" << endl;
}

void Client::handleMessage(const request_message& msg) {
    cout << "Client::handleMessage(const request_message& msg)" << endl;
}

void Client::handleMessage(const sync_request_message& msg) {
    cout << "Client::handleMessage(const sync_request_message& msg)" << endl;
}

void Client::handleMessage(const signed_block_ptr& msg) {
    cout << "Client::handleMessage(const signed_block_ptr& msg)" << endl;
}

void Client::handleMessage(const packed_transaction_ptr& msg) {
    cout << "Client::handleMessage(const packed_transaction_ptr& msg)" << endl;
}

void Client::handleMessage(const response_p2p_message& msg) {
    cout << "Client::handleMessage(const response_p2p_message& msg)" << endl;
}

void Client::handleMessage(const request_p2p_message& msg) {
    cout << "Client::handleMessage(const request_p2p_message& msg)" << endl;
}

void Client::StartReadMessage() {
    if(!_socket.is_open()) return;
    std::size_t minimum_read = _outStandingReadBytes ? *_outStandingReadBytes:message_header_size;
    auto completion_handler = [minimum_read](boost::system::error_code ec, std::size_t bytes_transferred) -> std::size_t {
        if(ec||bytes_transferred >= minimum_read)
            return 0;
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
                    }
                } catch (...) {
                    cerr << "Catch exception." << endl;
                    _socket.close();
                }
            });
}

void Client::StartSendTimeMessage() {
    DoSendTimeMessage();
}

void Client::DoSendTimeMessage() {
    if(!_socket.is_open()) return;
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

    char buffer[1024];
    fc::datastream<char *> ds(buffer, buffer_size);
    ds.write(header, header_size);
    fc::raw::pack(ds, n);

    _socket.async_write_some(
            boost::asio::buffer(buffer, buffer_size),
            [this](boost::system::error_code ec, std::size_t w){
                if(ec) {
                    cerr << "Error when asyn_write_some." << endl;
                    cerr << ec.message() << endl;
                    _socket.close();
                    return;
                }
                cout << "async write successfully." << endl;
            });
    _timer.expires_after(std::chrono::seconds(50));
    _timer.async_wait(std::bind(&Client::DoSendTimeMessage, this));
}

void Client::OnResolve(boost::system::error_code ec, tcp::resolver::results_type endpoints) {
    if(ec) {
        cerr << "Resolve error." << endl;
    } else {
        cout << "Resolve ok." << endl;
        auto handle = std::bind(&Client::OnConnect, this, std::placeholders::_1, std::placeholders::_2);
        boost::asio::async_connect(_socket, endpoints, handle);
    }
}


int main(void) {
    const char* host = "127.0.0.1";
    const char* port = "9877";

    boost::asio::io_service ioc;
    Client client(ioc, host, port);

    ioc.run();
    return 0;
}