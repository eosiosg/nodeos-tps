#!/usr/bin/env bash

# 第一次编译之前需要初始化编译环境
# 下载编译fc和boost

function usage() {
    echo "受如下参数(fc和boost编译之后的安装路径):
  bash before_compile.sh <fc-include-path> <fc-lib-path> <boost-root>"
}

function createDir() {
    mkdir -p ${1}
}

if [[ $# -ne 3 ]];then
    usage
    exit 1
fi

FC_INCLUDE=${1}
FC_LIB=${2}
BOOST_ROOT=${3}

createDir ${FC_INCLUDE}
createDir ${FC_LIB}
createDir ${BOOST_ROOT}


# --------- fc install ----------

if [[ ! -d fc ]];then
    git clone https://github.com/EOSIO/fc.git --recursive
    [[ $? -ne 0 ]] && echo "git clone https://github.com/EOSIO/fc.git --recursive failed." && rm -rf fc && exit 1
fi

cd fc
mkdir build
cd build
cmake ../
[[ $? -ne 0 ]] && echo "compile cmake fc failed." && exit 1
make
[[  $? -ne 0 ]] && echo "compile make fc failed." && exit 1
cp -r ../include/fc ${FC_INCLUDE}/
cp libfc.a ${FC_LIB}/

# --------- fc install end----------

# --------- boost install ----------
if [[ ! -d boost ]];then
    STATUS=$(curl -LO -w '%{http_code}' --connect-timeout 30 https://dl.bintray.com/boostorg/release/1.67.0/source/boost_1_67_0.tar.bz2)
    if [[ "${STATUS}" -ne 200 ]];then
        echo "Download boost 1.67 failed."
        exit 1
    fi
    if ! tar xf "boost_1_67_0.tar.bz2"
    then
        echo "unarchive boost_1_67_0.tar.bz2 failed."
        exit 1
    fi
    cd boost_1_67_0/
    ./bootstrap.sh "--prefix=${BOOST_ROOT}"
    [[ $? -ne 0 ]] && echo "Build boost 1.67 failed." && exit 1
    ./b2 install
    [[ $? -ne 0 ]] && echo "Install boost 1.67 failed."  && exit 1
fi

