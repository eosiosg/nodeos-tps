cmake_minimum_required(VERSION 3.12)

project(nodeos-tps)

set(CMAKE_CXX_STANDARD 17)

#add_subdirectory(hello)
#add_subdirectory(test)
#add_subdirectory(client)

find_package(Boost COMPONENTS system date_time filesystem chrono iostreams REQUIRED)

FILE(GLOB_RECURSE TEST_SRC "${CMAKE_SOURCE_DIR}/client/*.cpp" "${CMAKE_SOURCE_DIR}/client/*.h")

add_executable(nodeos-tps ${TEST_SRC})

target_link_libraries(nodeos-tps fc ssl crypto secp256k1 gmp pthread ${Boost_LIBRARIES})

install(TARGETS nodeos-tps
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static)