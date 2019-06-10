cmake_minimum_required(VERSION 3.14)
project(secp256k1)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
message("cmake_module_path: ${CMAKE_MODULE_PATH}")
find_package(GMP REQUIRED)

add_library(secp256k1 STATIC
  upstream/src/secp256k1.c
)

target_include_directories(secp256k1
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/upstream/
        ${CMAKE_CURRENT_SOURCE_DIR}/upstream/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/upstream/src
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${GMP_INCLUDE_DIR}
)

target_compile_definitions(secp256k1 PRIVATE HAVE_CONFIG_H=1)

target_link_libraries(secp256k1 ${GMP_LIBRARIES})
install( TARGETS secp256k1 
   RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_BINDIR}
   LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}
   ARCHIVE DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}
)
