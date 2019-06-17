aux_source_directory(. DIR_SRCS)
find_package(Boost COMPONENTS system date_time filesystem chrono iostreams REQUIRED)
add_executable(client ${DIR_SRCS})

target_link_libraries(client fc ssl crypto pthread ${Boost_LIBRARIES})