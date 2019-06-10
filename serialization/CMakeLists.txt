aux_source_directory(. DIR_LIB_SRCS)

add_library (serialization other_fc base58 ${DIR_LIB_SRCS})
