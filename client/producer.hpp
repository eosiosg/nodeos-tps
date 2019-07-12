//
// Created by zwg on 19-7-11.
//

#ifndef NODEOS_TPS_PRODUCER_HPP
#define NODEOS_TPS_PRODUCER_HPP

#include "client.hpp"
#include "tpsclient.hpp"

class Producer {
public:
    virtual ~Producer(){}
protected:
    virtual void sendMessage(packed_transaction&&) = 0;
};

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
    boost::asio::steady_timer timerSendTransferTransaction{IOC::app()};
    TestInfo(uint64_t period, uint32_t eachTime): batch(eachTime), timer_timeout(period) { };
    void update(const signed_block &msg) {
        this->head_block_id = msg.previous;
    }

};

class TpsProducer:public Producer{
private:
    std::list<OutQueue*> queueList;
    TestInfo testInfo;
    AsyncBufferPool& bufferPool;
    void startGeneration(const string& salt) {
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
    void sendTransferTransaction() {
        try {
            testInfo.timerSendTransferTransaction.expires_after(std::chrono::microseconds(testInfo.timer_timeout));
            testInfo.timerSendTransferTransaction.async_wait(std::bind(&TpsProducer::sendTransferTransaction, this));
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
        }catch (std::exception e) {
            cerr << "std::exception: " << e.what() << endl;
        }
    }
    bool checkQueueStatus(OutQueue& outQueue) {
        if(outQueue.size() >= 800) {
            //防御，防止内存沾满
            static uint64_t c = 0;
            if(c++%10000 == 0)
                cerr << "Warning:" <<"outQueue.size(" << outQueue.size() << ") > 800" << endl;
            return false;
        }
        return true;
    }
protected:
    void sendMessage(packed_transaction&& msg) {
        for(auto&x:queueList) {
            auto& outQueue = *x;
            if (!checkQueueStatus(outQueue)) return;
            net_message netMsg(msg);
            int32_t payload_size = fc::raw::pack_size(netMsg);
            char* header = reinterpret_cast<char*>(&payload_size);
            size_t header_size = sizeof(payload_size);
            size_t messageLen = header_size+payload_size;
            if (messageLen>outQueue.allocSize-outQueue.wIndex) {
                outQueue.push_back(Buffer(outQueue.pTemp, outQueue.wIndex));
                outQueue.pTemp = bufferPool.newBuffer(outQueue.allocSize);
                outQueue.wIndex = 0;
            }
            fc::datastream<uint8_t*> ds(outQueue.pTemp+outQueue.wIndex, messageLen);
            ds.write(header, header_size);
            fc::raw::pack(ds, netMsg);
            outQueue.wIndex += messageLen;
        }
    }
public:
    TpsProducer(
            const string& cid,
            const string& user1, const string& user1PK,
            const string& user2, const string& user2PK,
            const string& tokenName, const string& contractName,
            uint64_t period, uint32_t eachTime)
            :testInfo(period, eachTime),bufferPool(AsyncBufferPool::Instance()) {
        testInfo.chain_id = chain_id_type(cid);
        testInfo.user1 = name(user1);
        testInfo.user2 = name(user2);
        testInfo.user1PK = fc::crypto::private_key(user1PK);
        testInfo.user2PK = fc::crypto::private_key(user2PK);
        testInfo.tokenName = tokenName;
        testInfo.contractName = name(contractName);
        std::srand(std::time(nullptr));
        startGeneration("abcdefg");
    }
    virtual void Register(TpsClient& pc){
        queueList.emplace_back(pc.GetQueuePointer());
        pc.setUpdateVar(&testInfo.head_block_id);
    }
    virtual ~TpsProducer(){}
};

class StatProducer: public Producer{
    void Register(Client&){}
    void sendMessage(packed_transaction&&){}
};

#endif //NODEOS_TPS_PRODUCER_HPP
