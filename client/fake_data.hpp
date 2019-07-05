//
// Created by zwg on 19-6-12.
//

#ifndef NODEOS_TPS_FAKE_DATA_HPP
#define NODEOS_TPS_FAKE_DATA_HPP
# include "protocol.hpp"
#include <cstring>
#include <chrono>

using namespace std;
class FakeData {
public:
    static eosio::handshake_message fakeHandShakeMessage(eosio::chain_id_type cid) {
        eosio::handshake_message h;
        h.network_version = 1206;

        h.chain_id = cid;

        uint64_t node_id[4] = {
                16341233654929650305ul,
                6867449077418379323ul,
                6345568250656053691ul,
                12222374913718194331ul
        };
        memcpy(h.node_id._hash, node_id, sizeof(node_id));

        h.key = eosio::public_key_type();
        h.time = std::chrono::system_clock::now().time_since_epoch().count();
        h.token = fc::sha256::hash(h.time);
        h.sig = fc::crypto::signature();
        h.p2p_address = "127.0.0.1:9876 - 8172bdf";
        h.os = "linux";
        h.agent = "\"nodeos-tps\"";
        h.head_id = fc::sha256();
        h.head_num = 0;
        h.last_irreversible_block_num = 0;
        h.generation = 2;
        return h;
    }

    static eosio::handshake_message invalidFakeHandShakeMessage(eosio::chain_id_type cid) {
        auto h = fakeHandShakeMessage(cid);
        h.last_irreversible_block_num = UINT32_MAX;
        return h;
    }

    static eosio::handshake_message acceptHandshakeMessage(eosio::chain_id_type cid) {
        auto h = fakeHandShakeMessage(cid);
        h.generation = 1;
        return h;
    }
};

#endif //NODEOS_TPS_FAKE_DATA_HPP
