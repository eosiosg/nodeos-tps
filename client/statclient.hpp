//
// Created by zwg on 19-7-12.
//

#ifndef NODEOS_TPS_STATCLIENT_HPP
#define NODEOS_TPS_STATCLIENT_HPP

#include "client.hpp"
#include "grafana.hpp"
#include "timecostguard.hpp"

class StatClient: public Client {
private:
    Grafana grafana;
public:
    StatClient() = delete;
    StatClient(const string& host, const string& port,
               const string& grafanaHost, const string& grafanaPort):
            Client(host, port),grafana(grafanaHost, grafanaPort){ }

    virtual void toGrafana(MsgType type, std::size_t len) {
        TimeCostGuard tcg(output, "Client::toGrafana");
        stringstream ss;
        ss << "msgstat,m=" << type << ",hp=" << host << ":" << port
           << " len=" << len << " " << fc::time_point::now().time_since_epoch().count()*1000;
        auto p = ss.str();
        grafana.send(p.c_str(), p.length());
    }
    bool processNextMessage(uint32_t messageLen) override {
        try {
            auto ds = _messageBuffer.create_datastream();
            net_message msg;
            fc::raw::unpack(ds, msg);
            MsgHandler msgHandler(this);
            if (msg.contains<signed_block>()) msgHandler(std::move(msg.get<signed_block>()));
            else if (msg.contains<packed_transaction>()) msgHandler(std::move(msg.get<packed_transaction>()));
            else msg.visit(msgHandler);

            if (msg.contains<handshake_message>()) toGrafana(MsgType::HANDSHAKE, messageLen);
            else if (msg.contains<chain_size_message>()) toGrafana(MsgType::CHAIN_SIZE, messageLen);
            else if (msg.contains<go_away_message>()) toGrafana(MsgType::GO_AWAY, messageLen);
            else if (msg.contains<time_message>()) toGrafana(MsgType::TIME, messageLen);
            else if (msg.contains<notice_message>()) toGrafana(MsgType::NOTICE, messageLen);
            else if (msg.contains<request_message>()) toGrafana(MsgType::REQUEST, messageLen);
            else if (msg.contains<sync_request_message>()) toGrafana(MsgType::SYNC_REQUEST, messageLen);
            else if (msg.contains<signed_block>()) toGrafana(MsgType::SIGNED_BLOCK, messageLen);
            else if (msg.contains<packed_transaction>()) toGrafana(MsgType::PACKED_TRANSACTION, messageLen);
            else if (msg.contains<response_p2p_message>()) toGrafana(MsgType::CHAIN_SIZE, messageLen);
            else if (msg.contains<request_p2p_message>()) toGrafana(MsgType::REQUEST_P2P, messageLen);
            else if (msg.contains<pbft_prepare>()) toGrafana(MsgType::PBFT_PREPARE, messageLen);
            else if (msg.contains<pbft_commit>()) toGrafana(MsgType::PBFT_COMMIT, messageLen);
            else if (msg.contains<pbft_view_change>()) toGrafana(MsgType::PBFT_VIEW_CHANGE, messageLen);
            else if (msg.contains<pbft_new_view>()) toGrafana(MsgType::PBFT_NEW_VIEW, messageLen);
            else if (msg.contains<pbft_checkpoint>()) toGrafana(MsgType::PBFT_CHECKPOINT, messageLen);
            else if (msg.contains<pbft_stable_checkpoint>()) toGrafana(MsgType::PBFT_STABLE_CHECKPOINT, messageLen);
            else if (msg.contains<checkpoint_request_message>()) toGrafana(MsgType::CHECKPOINT_REQUEST, messageLen);
            else if (msg.contains<compressed_pbft_message>()) toGrafana(MsgType::COMPRESSED_PBFT, messageLen);
            else {
                cerr << "Invalid message type" << endl;
                return false;
            }
        } catch(const fc::exception& e) {
            cerr << "fc::exception :" << e.what() << endl;
            return false;
        }
        return true;
    }
};

#endif //NODEOS_TPS_STATCLIENT_HPP