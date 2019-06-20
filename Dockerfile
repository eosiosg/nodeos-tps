FROM ubuntu:18.10

#ADD ./sources.list /etc/apt/sources.list
RUN apt update -y && apt install -y vim 
RUN apt install -y clang
RUN apt install -y cmake
RUN apt remove -y gcc
RUN apt install -y wget git curl

RUN apt install -y libgmp-dev libgmp3-dev libssl-dev
RUN apt install -y libboost1.67-all-dev

RUN git clone https://github.com/EOSIO/fc.git --recursive

RUN cd fc && mkdir build && cd build && cmake ../ && make -j && make install

# nodeos-tps source code compile
RUN git clone https://github.com/eosiosg/nodeos-tps.git
RUN cd nodeos-tps && git checkout feature/test_client_for_connect_nodeos
RUN mkdir -p nodeos-tps/build && cd nodeos-tps/build && cmake ../ -DCMAKE_INSTALL_PREFIX=/usr && make -j && make install

