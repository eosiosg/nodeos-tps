//
// Created by zwg on 19-6-6.
//

#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <chrono>
#include "serialization.hpp"
#include "other_fc.hpp"
using namespace std;

TEST(SerializationTest, Hello) {
    cout << "Hello, world!" << endl;
}

TEST(SerializationTest, PackUnpackTest) {
    struct A { string a; };

    struct B{ A a; int b; long long c; A d; };
    enum VType { AA, BB };

    REGISTER_NO_CALLABLE_CLASS_1(A, a);
    REGISTER_NO_CALLABLE_CLASS_4(B, a, b, c, d);
    B x{{"ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789"}, 3, 5, {"Simple"}};

    char buffer[200];
    memset(buffer, 0xff, 200);
    VType p = BB;
    sz::Serialization<B, VType> serialization;

    serialization.setBuffer(buffer, 200);

    auto ret = serialization.pack(x, p);
    EXPECT_EQ(ret.first, true);

    B y;
    VType q;
    sz::Deserialization<B, VType> deserialization;
    deserialization.setBuffer(buffer, ret.second);
    auto ret2 = deserialization.unpack(y, q);
    EXPECT_EQ(ret2.first, true);
    EXPECT_EQ(ret2.second, 150);
    EXPECT_EQ(q, p);
    EXPECT_EQ(x.a.a, y.a.a);
    EXPECT_EQ(x.b, y.b);
    EXPECT_EQ(x.c, y.c);
    EXPECT_EQ(x.d.a, y.d.a);
}

TEST(SerializationTest, sha256Test) {
    REGISTER_CALLABLE_CLASS(sha256, sha256::serialize, sha256::deserialize);
    sha256 x("aca376f206b8fc25a6ed44dbdc66547c36c6c33e3a119ffbeaef943642f0e906");
    EXPECT_EQ(x.hash[0], 2737265013511594924ul);
    EXPECT_EQ(x.hash[1], 8958898657504325030ul);
    EXPECT_EQ(x.hash[2], 18131229566675699254ul);
    EXPECT_EQ(x.hash[3], 498193400975388650ul);

    enum VType{XX, YY};

    unsigned char expect_buffer[100]={1,0xac,0xa3,0x76,0xf2,0x06,0xb8,0xfc,0x25,0xa6,0xed,0x44,0xdb,0xdc,0x66,0x54,0x7c,0x36,0xc6,0xc3,0x3e,
                             0x3a,0x11,0x9f,0xfb,0xea,0xef,0x94,0x36,0x42,0xf0,0xe9,0x06};
    memset(expect_buffer + 33, 0xff, 100);


    unsigned char buffer[100];

    memset(buffer, 0xff, 100);
    sz::Serialization<sha256, VType> serialization;
    serialization.setBuffer(buffer, 100);
    auto rt1 = serialization.pack(x, YY);

    //serialization
    EXPECT_EQ(rt1.first, true);
    EXPECT_EQ(rt1.second, 33);
    for(int i=0; i < 100; i++) {
        EXPECT_EQ(buffer[i], expect_buffer[i]);
    }

    VType b;
    sha256 y;

    sz::Deserialization<sha256, VType> deserialization;
    deserialization.setBuffer(buffer, 100);
    auto rt2 = deserialization.unpack(y, b);
    EXPECT_EQ(rt2.first, true);
    EXPECT_EQ(rt2.second, 33);
    EXPECT_EQ(b, YY);
    EXPECT_EQ(y.hash[0], 2737265013511594924ul);
    EXPECT_EQ(y.hash[1], 8958898657504325030ul);
    EXPECT_EQ(y.hash[2], 18131229566675699254ul);
    EXPECT_EQ(y.hash[3], 498193400975388650ul);
}

TEST(SerializationTest, PublicKeyTest) {
    REGISTER_CALLABLE_CLASS(public_key, public_key::serialize, public_key::deserialize);
    enum VType{XX, YY, ZZ};
    public_key key("EOS5vZYfat26kNXMbhvy2WX3Sy1zA3rxi79Ludpnrh4PPUBdJMTBB");
    unsigned char buffer[100];
    memset(buffer, 0xff, 100);

    unsigned char expect_buffer[100] = {2, 0, 2, 0x88, 102, 0xc8, 102, 0x9c, 23, 110, 0x87, 88,
                                        17, 0x9b, 0, 96, 58, 0xdf, 63, 16, 0xae, 111, 110, 0xae,
                                        0xa2, 0xf8, 0xfa, 0xed, 120, 96, 0x92, 52, 0xd3, 0xf8, 80};
    memset(expect_buffer+35, 0xff, 65);

    sz::Serialization<public_key, VType> serialization;
    serialization.setBuffer(buffer, 100);
    auto rt1 = serialization.pack(key, ZZ);
    EXPECT_EQ(rt1.first, true);
    EXPECT_EQ(rt1.second, 35);
    for(int i=0;i<100;i++) {
        EXPECT_EQ(buffer[i], expect_buffer[i]);
    }

    VType pp;
    public_key x;
    sz::Deserialization<public_key, VType> deserialization;
    deserialization.setBuffer(buffer, 100);
    auto rt2 = deserialization.unpack(x, pp);
    EXPECT_EQ(rt2.first, true);
    EXPECT_EQ(rt2.second, 35);
    EXPECT_EQ(x.storage, key.storage);
}

//继承
TEST(SerializationTest, ExtendsTest) {
    struct A { string a; };
    struct B{ A a; int b; long long c; A d; };
    struct C:public B { A e; string f;};
    REGISTER_NO_CALLABLE_CLASS_1(A, a);
    REGISTER_NO_CALLABLE_CLASS_4(B, a, b, c, d);
    REGISTER_NO_CALLABLE_CLASS_6(C, a, b, c, d, e, f);

    enum VType {XXX, YYY, ZZZ};

    C x;
    x.a.a = "Hello";
    x.b = 2;
    x.c = 3ll;
    x.d.a = "China";
    x.e.a = "Meto";
    x.f = "Waiting";
    unsigned char expect_buffer[] = {(unsigned char)ZZZ,
                                     5, 'H', 'e', 'l', 'l', 'o',
                                     2, 0, 0, 0,
                                     3, 0, 0, 0, 0, 0, 0, 0,
                                     5, 'C', 'h', 'i', 'n', 'a',
                                     4, 'M', 'e', 't', 'o',
                                     7, 'W', 'a', 'i', 't', 'i', 'n', 'g',
                                     0xff, 0xff, 0xff, 0xff};
    unsigned char buffer[100];
    memset(buffer, 0xff, 100);
    sz::Serialization<C, VType> serialization;
    serialization.setBuffer(buffer, 100);
    auto rt1 = serialization.pack(x, ZZZ);
    EXPECT_EQ(rt1.first, true);
    EXPECT_EQ(rt1.second, sizeof(expect_buffer) -4);
    for(auto i=0;i< sizeof(expect_buffer); i++) {
        EXPECT_EQ(expect_buffer[i], buffer[i]);
    }

    sz::Deserialization<C, VType> deserialization;
    deserialization.setBuffer(buffer, 100);
    C p;
    VType m;
    auto rt2 = deserialization.unpack(p, m);
    EXPECT_EQ(rt2.first, true);
    EXPECT_EQ(rt2.second, sizeof(expect_buffer) - 4);
    EXPECT_EQ(m, ZZZ);
    EXPECT_EQ(p.a.a, x.a.a);
    EXPECT_EQ(p.b, x.b);
    EXPECT_EQ(p.c, x.c);
    EXPECT_EQ(p.d.a, x.d.a);
    EXPECT_EQ(p.e.a, x.e.a);
    EXPECT_EQ(p.f, x.f);
}

//handshake_message除去signature之外的信息
TEST(SerializationTest, HandshakeMessageWithoutSigTest) {
    REGISTER_CALLABLE_CLASS(sha256, sha256::serialize, sha256::deserialize);
    REGISTER_CALLABLE_CLASS(public_key, public_key::serialize, public_key::deserialize);
    typedef chrono::system_clock::duration::rep tstamp;
    struct handshake_message {
        uint16_t network_version = 0;
        sha256 chain_id;
        sha256 node_id;
        public_key key;
        tstamp time;
        sha256 token;
        //fc::crypto::signature sig;
        string p2p_address;
        uint32_t last_irreversible_block_num = 0;
        sha256 last_irreversible_block_id;
        uint32_t head_num = 0;
        sha256 head_id;
        string os;
        string agent;
        int16_t generation;
    };
    REGISTER_NO_CALLABLE_CLASS_14(handshake_message, network_version, chain_id, node_id, key, time, token,
                                 p2p_address, last_irreversible_block_num, last_irreversible_block_id,
                                 head_num, head_id, os, agent, generation);

    auto h = handshake_message{
            .network_version=666,
            .chain_id=sha256("aca376f206b8fc25a6ed48736c66547c36c6c33e3a119ffbeaef943642f0e906"),
            .node_id=sha256("aca376f206b8fc25a6ed44cccc66547c36c6c33e3a119ffbeaef943642f0e906"),
            .key=public_key("EOS5vZYfat26kNXMbhvy2WX3Sy1zA3rxi79Ludpnrh4PPUBdJMTBB"),
            .time=1559911361,
            .token=sha256("aca376f206b8fc25a6ed44dbdc66547c36c6c33e3a119ffbeaef943642f0e906"),
            .p2p_address="Localhost 1927 9876",
            .last_irreversible_block_num=20,
            .last_irreversible_block_id=sha256("aca376f206b8fc25a6ed44dbdc66547c36c6c33e3a119ffbeaef943642f01234"),
            .head_id=sha256("aca3737206b8fc25a6ed44dbdc66547c36c6c33e3a119ffbeaef943642f01234"),
            .os="linux ubuntu",
            .agent="Super Gang",
            .generation=20};

    enum VType {XXX, YYY, ZZZ};
    unsigned char expected_buffer[] = {0x2,0x9a,0x2,0xac,0xa3,0x76,0xf2,0x6,0xb8,0xfc,0x25,0xa6,0xed,0x48,0x73,0x6c,
                                       0x66,0x54,0x7c,0x36,0xc6,0xc3,0x3e,0x3a,0x11,0x9f,0xfb,0xea,0xef,0x94,0x36,
                                       0x42,0xf0,0xe9,0x6,0xac,0xa3,0x76,0xf2,0x6,0xb8,0xfc,0x25,0xa6,0xed,0x44,0xcc,
                                       0xcc,0x66,0x54,0x7c,0x36,0xc6,0xc3,0x3e,0x3a,0x11,0x9f,0xfb,0xea,0xef,0x94,
                                       0x36,0x42,0xf0,0xe9,0x6,0x0,0x2,0x88,0x66,0xc8,0x66,0x9c,0x17,0x6e,0x87,0x58,
                                       0x11,0x9b,0x0,0x60,0x3a,0xdf,0x3f,0x10,0xae,0x6f,0x6e,0xae,0xa2,0xf8,0xfa,0xed,
                                       0x78,0x60,0x92,0x34,0xd3,0xf8,0x50,0xc1,0x5b,0xfa,0x5c,0x0,0x0,0x0,0x0,0xac,
                                       0xa3,0x76,0xf2,0x6,0xb8,0xfc,0x25,0xa6,0xed,0x44,0xdb,0xdc,0x66,0x54,0x7c,0x36,
                                       0xc6,0xc3,0x3e,0x3a,0x11,0x9f,0xfb,0xea,0xef,0x94,0x36,0x42,0xf0,0xe9,0x6,0x13,
                                       0x4c,0x6f,0x63,0x61,0x6c,0x68,0x6f,0x73,0x74,0x20,0x31,0x39,0x32,0x37,0x20,0x39,
                                       0x38,0x37,0x36,0x14,0x0,0x0,0x0,0xac,0xa3,0x76,0xf2,0x6,0xb8,0xfc,0x25,0xa6,0xed,
                                       0x44,0xdb,0xdc,0x66,0x54,0x7c,0x36,0xc6,0xc3,0x3e,0x3a,0x11,0x9f,0xfb,0xea,0xef,
                                       0x94,0x36,0x42,0xf0,0x12,0x34,0x0,0x0,0x0,0x0,0xac,0xa3,0x73,0x72,0x6,0xb8,0xfc,
                                       0x25,0xa6,0xed,0x44,0xdb,0xdc,0x66,0x54,0x7c,0x36,0xc6,0xc3,0x3e,0x3a,0x11,0x9f,
                                       0xfb,0xea,0xef,0x94,0x36,0x42,0xf0,0x12,0x34,0xc,0x6c,0x69,0x6e,0x75,0x78,0x20,
                                       0x75,0x62,0x75,0x6e,0x74,0x75,0xa,0x53,0x75,0x70,0x65,0x72,0x20,0x47,0x61,0x6e,
                                       0x67,0x14,0x0,
                                       0xff, 0xff, 0xff};

    unsigned char buffer[300];
    memset(buffer, 0xff, 300);

    sz::Serialization<handshake_message, VType> serialization;
    serialization.setBuffer(buffer, 300);
    auto rt1 = serialization.pack(h, ZZZ);
    EXPECT_EQ(rt1.first, true);
    EXPECT_EQ(rt1.second, sizeof(expected_buffer) - 3);
    for(auto i=0; i < sizeof(expected_buffer); i++)
        EXPECT_EQ(expected_buffer[i], buffer[i]);

    sz::Deserialization<handshake_message, VType> deserialization;
    deserialization.setBuffer(buffer, 300);
    VType vType;
    handshake_message hm;
    auto rt2 = deserialization.unpack(hm, vType);
    EXPECT_EQ(rt2.first, true);
    EXPECT_EQ(rt2.second, sizeof(expected_buffer) - 3);
    EXPECT_EQ(vType, ZZZ);
    EXPECT_EQ(h.network_version, hm.network_version);
    EXPECT_TRUE(h.chain_id == hm.chain_id);
    EXPECT_TRUE(h.node_id == hm.node_id);
    EXPECT_TRUE(h.key == hm.key);
    EXPECT_EQ(h.time, hm.time);
    EXPECT_TRUE(h.token == hm.token);
    EXPECT_EQ(h.p2p_address, hm.p2p_address);
    EXPECT_EQ(h.last_irreversible_block_num, hm.last_irreversible_block_num);
    EXPECT_TRUE(h.last_irreversible_block_id == hm.last_irreversible_block_id);
    EXPECT_EQ(h.head_num, hm.head_num);
    EXPECT_TRUE(h.head_id == hm.head_id);
    EXPECT_EQ(h.os, hm.os);
    EXPECT_EQ(h.agent, hm.agent);
    EXPECT_EQ(h.generation, hm.generation);
}