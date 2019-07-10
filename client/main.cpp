//
// Created by zwg on 19-7-10.
//

#include "main.hpp"

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
    const char* grafanaIP = argv[4];
    const char* grafanaPort = argv[5];
    while(true) {
        boost::asio::io_service ioc;
        Client client(ioc, host, port, grafanaIP, grafanaPort);
        ioc.run();
        cout << "Reconnect." << endl;
    }
}

int main(int argc, char* argv[]) {
    cout << argc << endl;
    for(auto i = 1; i < argc; i++) {
        cout << argv[i] << endl;
    }
    if(argc != 13 and argc != 6) {
        cerr << "Invalid parameter." << endl;
        cerr << "Usage: ./nodeos-tps test-tps ip port chain_id user1 private_key_of_user1 user2 private_key_of_user2 token_name contract_name microseconds_interval count_for_each" << endl;
        cerr << "Example: ./nodeos-tps test-tps 127.0.0.1 9876 "
             << "cf057bbfb72640471fd910bcb67639c22df9f92470936cddc1ade0e2f2e7dc4f "
             << "aaaaaaaaaaaa 5HsvPQ2wBttkMMYXfJUw2QW5pYh5ReSqVBqPhprWh3GGhiQyezC "
             << "bbbbbbbbbbbb 5JAghZg5An1L8DdT75CyQSaZAHuofY9mst52oCW9gQUQjs1n76L "
             << "BOS eosio.token 1000 2" << endl;
        cerr << "Usage: ./nodeos-tps stat ip port grafanaIP grafanaPort" << endl;
        cerr << "Example: ./nodeos-tps stat 127.0.0.1 9876 127.0.0.1 8089" << endl;
        exit(1);
    }

    const char* feature = argv[1];
    if(strcmp(feature, "test-tps") == 0) testTps(argv);
    else if (strcmp(feature, "stat") == 0) stat(argv);
    else cerr << "Invalid parameter." << endl;
    return 1;
}