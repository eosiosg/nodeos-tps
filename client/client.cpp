//
// Created by zwg on 19-6-12.
//
#include "client.hpp"
#include "outbuffer.hpp"
#include "stat.hpp"

MsgHandler::MsgHandler(Client* p):_pCli(p) { }

void Client::OnConnect(boost::system::error_code ec, tcp::endpoint endpoint) {
    canCheckStatus = true;
    if (ec) {
        cerr << "Connect failed: " << ec.message() << std::endl;
        connect = false;
        return;
    }
    connect = true;
    StartSendTimeMessage();
    StartReadMessage();
    StartHandshakeMessage();
    StartSendoutData();
}

void Client::sendHandshakeMessage(handshake_message && msg) {
   sendMessage(msg);
}

void Client::StartHandshakeMessage(void) {
    auto msg = FakeData::fakeHandShakeMessage();
    sendHandshakeMessage(std::move(msg));
}

void Client::sendMessage(packed_transaction&& msg) {
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
        cerr << "fc::exception :" << e.what() << endl;
        return false;
    }
    return true;
}

void Client::Reconnect(void) {
    output << "connect to " << host << ":" << port << endl;
    if(_socket.is_open()) _socket.close();
    connect = false;
    canCheckStatus = false;
    timerMakePeerSync = boost::asio::steady_timer{IOC::app()};
    timerSendTimeMessage = boost::asio::steady_timer{IOC::app()};
    timerOutQueue = boost::asio::steady_timer{IOC::app()};
    _resolver.async_resolve(tcp::v4(), host, port,
            std::bind(&Client::OnResolve, this, std::placeholders::_1, std::placeholders::_2));
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

void Client::handleMessage(const notice_message& msg) {
    // 让对端认为数据已经同步(将sync->true)
    timerMakePeerSync.expires_after(std::chrono::milliseconds(500));
    timerMakePeerSync.async_wait(std::bind(&Client::makePeerSync, this));
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
                                connect = false;
                                return;
                            }
                            auto totalMessageLength = messageLength + message_header_size;
                            //当前缓存区已经有本条完整的数据
                            if(bytesInBuffer >= totalMessageLength) {
                                _messageBuffer.advance_read_ptr(message_header_size);
                                if(!processNextMessage(messageLength)) {
                                    _socket.close();
                                    connect = false;
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
                        connect = false;
                        return;
                    }
                } catch (...) {
                    cerr << "Catch exception." << endl;
                    _socket.close();
                    connect = false;
                    return;
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
            timerOutQueue.expires_after(std::chrono::microseconds(outQueue.microsEmptyWaitTime));
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
                        connect = false;
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
        connect = false;
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
        canCheckStatus = true;
        connect = false;
    } else {
        output << "Resolve ok." << endl;
        auto handle = std::bind(&Client::OnConnect, this, std::placeholders::_1, std::placeholders::_2);
        boost::asio::async_connect(_socket, endpoints, handle);
    }
}
