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
    static eosio::handshake_message fakeHandShakeMessage() {
        eosio::handshake_message h;
        h.network_version = 1206;

        uint64_t chain_id[4] = {
                5134146145835746767ul,
                13995347844241873183ul,
                15955289689732806957ul,
                5754729454247128513ul
        };
        memcpy(h.chain_id._hash, chain_id, sizeof(chain_id));

        uint64_t node_id[4] = {
                16341233654929650305ul,
                6867449077418379323ul,
                6345568250656053691ul,
                12222374913718194331ul
        };
        memcpy(h.node_id._hash, chain_id, sizeof(node_id));

        h.key = eosio::public_key_type();
        h.time = std::chrono::system_clock::now().time_since_epoch().count();
        h.token = fc::sha256::hash(h.time);
        h.sig = fc::crypto::signature();
        h.p2p_address = "127.0.0.1:9876 - 8172bdf";
        h.os = "linux";
        h.agent = "\"EOS Test Agent\"";
        h.head_id = fc::sha256();
        h.head_num = 0;
        h.last_irreversible_block_num = 0;
        h.generation = 2;
        return h;
    }

    static eosio::handshake_message invalidFakeHandShakeMessage() {
        auto h = fakeHandShakeMessage();
        h.last_irreversible_block_num = UINT32_MAX;
        return h;
    }

    static eosio::handshake_message acceptHandshakeMessage() {
        auto h = fakeHandShakeMessage();
        h.generation = 1;
        return h;
    }
};

#endif //NODEOS_TPS_FAKE_DATA_HPP
