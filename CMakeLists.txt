cmake_minimum_required(VERSION 3.12)

project(nodeos-tps)

set(CMAKE_CXX_STANDARD 17)

#add_subdirectory(hello)
#add_subdirectory(test)
#add_subdirectory(client)

find_package(Boost COMPONENTS system date_time filesystem chrono iostreams REQUIRED)

FILE(GLOB_RECURSE SRC "${CMAKE_SOURCE_DIR}/client/*.cpp" "${CMAKE_SOURCE_DIR}/client/*.hpp")
message(${SRC})
add_executable(nodeos-tps ${SRC} client/eosio.token.api.hpp client/abi_def.hpp client/abi_serializer.hpp client/exceptions.hpp client/action.hpp client/action_receipt.hpp client/block.hpp client/trace.hpp client/block_timestamp.hpp client/abi_serializer.cpp client/symbol.hpp client/core_symbol.hpp client/database_utils.hpp client/softfloat.hpp client/softfloat_types.hpp client/chainbase.hpp client/chainbase.cpp client/asset.hpp client/asset.cpp client/authority.hpp client/transaction.hpp client/config.hpp client/config_xos.hpp client/contract_types.hpp client/name.hpp client/eosio.system.abi.hpp client/transaction.cpp client/eosio.token.wast.hpp client/BasicTypes.hpp client/connectionpool.hpp client/outbuffer.hpp)

target_link_libraries(nodeos-tps fc ssl crypto secp256k1 gmp pthread ${Boost_LIBRARIES})

install(TARGETS nodeos-tps
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static)