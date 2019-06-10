configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
    message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
    message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
        ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
        EXCLUDE_FROM_ALL)

include_directories(../serialization)
# Now simply link against gtest or gtest_main as needed. Eg
add_executable(test main.cpp
        ../hello/hello.hpp
        ../hello/hello.cpp
        ../hello/hello_test.cpp
        ../serialization/serialization.cpp
        ../serialization/serialization.hpp
        ../serialization/other_fc.hpp
        ../serialization/other_fc.cpp
        ../serialization/base58.hpp
        ../serialization/base58.cpp
        serialization/serialization_test.cpp)

target_link_libraries(test gmp pthread ssl crypto gtest_main gmock_main)
add_test(NAME test_main COMMAND test)

#find_package(Boost 1.6.9 REQUIRED system)
#target_include_directories(test PUBLIC ${Boost_INCLUDE_DIRS})
#target_link_libraries(test ${Boost_LIBRARIES})