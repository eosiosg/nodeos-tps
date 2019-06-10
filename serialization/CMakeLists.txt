aux_source_directory(. DIR_LIB_SRCS)

include(../secp256k1/upstream/include)
add_library (serialization other_fc base58 ${DIR_LIB_SRCS})
