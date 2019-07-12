//
// Created by zwg on 19-7-10.
//
#include "main.hpp"

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