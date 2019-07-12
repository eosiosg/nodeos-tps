//
// Created by zwg on 19-7-10.
//

#include "main.hpp"

/*void testTps(char* argv[]) {
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
        auto& ioc = IOC::app();
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
        auto& ioc = IOC::app();
        Client client(ioc, host, port, grafanaIP, grafanaPort);
        ioc.run();
        cout << "Reconnect." << endl;
    }
}*/

int main(int argc, char* argv[]) {
    if(argc != 2 or strcmp(argv[1], "-h") == 0 or strcmp(argv[1], "--help") == 0) {
        cerr << "Usage: nodeos-tps ConfigFile" << endl;
        return 1;
    }
    ConfigParser configParser(argv[1]);
    std::unique_ptr<TpsProducer> pTpsProducer;
    ClientPool cpool{5000};

    if(configParser.tps) {
        pTpsProducer= std::make_unique<TpsProducer>(configParser.chainId,
                configParser.user1, configParser.user1PrivateKey,
                configParser.user2, configParser.user2PrivateKey,
                configParser.tokenName, configParser.contractName,
                configParser.microSeconds, configParser.eachTime);
        auto size = configParser.tpsHosts.size();
        for(auto i = 0; i < size; i ++) {
            auto p = std::make_unique<TpsClient>(configParser.tpsHosts[i], configParser.tpsPorts[i]);
            pTpsProducer->Register(*p);//注册获取生产者数据
            cpool.newClient(std::move(p));//注册到ClientPool(负责管理自动重连等)
        }
    }
    if(configParser.stat) {
        auto size = configParser.statHosts.size();
        for(auto i = 0; i < size; i++) {
            auto p = std::make_unique<StatClient>(
                    configParser.statHosts[i], configParser.statPorts[i],
                    configParser.influxdbHost, configParser.influxdbPort);
            cpool.newClient(std::move(p));
        }
    }
    IOC::app().run();
    return 1;
}