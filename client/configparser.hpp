//
// Created by zwg on 19-7-12.
//

#ifndef NODEOS_TPS_CONFIGPARSER_HPP
#define NODEOS_TPS_CONFIGPARSER_HPP

#include <string>
#include <fstream>
#include <vector>
#include <cstring>
using namespace std;

class FileException:public std::exception {
    const char* errorMsg;
public:
    FileException(const char* msg):errorMsg(msg) {}
    const char* what() const noexcept { return errorMsg;}
};

class ConfigParser {
private:
    string fname;
    ifstream file;
    char buff[1024];
public:// tps
    bool tps{false};
    string chainId;
    string user1;
    string user1PrivateKey;
    string user2;
    string user2PrivateKey;
    string tokenName; //EOS, BOS,....
    string contractName;
    vector<string> tpsHosts;
    vector<string> tpsPorts;
    uint64_t microSeconds; //每隔microSeconds微秒发送一次数据
    uint64_t eachTime;//每次数据发送eachTime条
public://stat
    bool stat{false};
    string influxdbHost;
    string influxdbPort;
    vector<string> statHosts;
    vector<string> statPorts;
public:
    ConfigParser(const char* filename) {
        fname = string(filename);
        file.open(filename);
        if(!file.is_open())
            throw FileException((string("Open config file(") + fname + string(") failed.")).c_str());
        while(!file.eof()) {
            file.getline(buff, 1024);
            if(strncmp(buff, "tps",3) == 0){
                tps = true;
                file.getline(buff, 1024); chainId = string(buff);
                file.getline(buff, 1024); user1 = string(buff);
                file.getline(buff, 1024); user1PrivateKey = string(buff);
                file.getline(buff, 1024); user2 = string(buff);
                file.getline(buff, 1024); user2PrivateKey = string(buff);
                file.getline(buff, 1024); tokenName = string(buff);
                file.getline(buff, 1024); contractName = string(buff);
                file.getline(buff, 1024); microSeconds = atol(buff);
                file.getline(buff, 1024); eachTime = atol(buff);
                file.getline(buff, 1024); int count = atoi(buff);
                for(int i = 0; i < count; i++){
                    file.getline(buff, 1024); tpsHosts.emplace_back(string(buff));
                    file.getline(buff, 1024); tpsPorts.emplace_back(string(buff));
                }
            } else if(strncmp(buff, "stat", 4) == 0) {
                stat = true;
                file.getline(buff, 1024); influxdbHost = string(buff);
                file.getline(buff, 1024); influxdbPort = string(buff);
                file.getline(buff, 1024); int count = atoi(buff);
                for(int i = 0; i < count; i++){
                    file.getline(buff, 1024); statHosts.emplace_back(string(buff));
                    file.getline(buff, 1024); statPorts.emplace_back(string(buff));
                }
            }
        }
    }
    ~ConfigParser() {
        if(file.is_open()) {
            file.close();
        }
    }
};

#endif //NODEOS_TPS_CONFIGPARSER_HPP
