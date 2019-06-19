//
// Created by zwg on 19-6-18.
//

#pragma once

#include <fc/container/flat_fwd.hpp>
#include <fc/io/varint.hpp>
#include <fc/io/enum_type.hpp>
#include <fc/crypto/sha224.hpp>
#include <fc/optional.hpp>
#include <fc/safe.hpp>
#include <fc/container/flat.hpp>
#include <fc/string.hpp>
#include <fc/io/raw.hpp>
#include <fc/static_variant.hpp>
#include <fc/smart_ref_fwd.hpp>
#include <fc/crypto/ripemd160.hpp>
#include <fc/fixed_string.hpp>
#include <fc/crypto/private_key.hpp>
#include <fc/static_variant.hpp>
#include <fc/time.hpp>
#include <fc/container/flat_fwd.hpp>
#include <fc/io/varint.hpp>
#include <fc/io/enum_type.hpp>
#include <fc/crypto/sha224.hpp>
#include <fc/optional.hpp>
#include <fc/safe.hpp>
#include <fc/container/flat.hpp>
#include <fc/string.hpp>
#include <fc/io/raw.hpp>
#include <fc/static_variant.hpp>
#include <fc/smart_ref_fwd.hpp>
#include <fc/crypto/ripemd160.hpp>
#include <fc/fixed_string.hpp>
#include <fc/crypto/private_key.hpp>
#include <fc/time.hpp>
#include <fc/bitutil.hpp>

#define EOS_ASSERT( expr, exc_type, FORMAT, ... )                \
   FC_MULTILINE_MACRO_BEGIN                                           \
   if( !(expr) )                                                      \
      FC_THROW_EXCEPTION( exc_type, FORMAT, __VA_ARGS__ );            \
   FC_MULTILINE_MACRO_END

FC_DECLARE_EXCEPTION( chain_exception, 3000000, "blockchain exception" )
FC_DECLARE_DERIVED_EXCEPTION( plugin_exception, chain_exception, 3110000, "Plugin exception" )
FC_DECLARE_DERIVED_EXCEPTION( plugin_config_exception, plugin_exception, 3110006, "Incorrect plugin configuration" )
FC_DECLARE_DERIVED_EXCEPTION( chain_type_exception, chain_exception, 3010000, "chain type exception" )
FC_DECLARE_DERIVED_EXCEPTION( name_type_exception, chain_type_exception, 3010001, "Invalid name" )

namespace eosio {

    using block_id_type       = fc::sha256;
    using public_key_type  = fc::crypto::public_key;
    using signature_type   = fc::crypto::signature;
    using transaction_id_type = fc::sha256;
    using block_num_type      = uint32_t;
    using pbft_view_type = uint32_t;
    struct chain_id_type : public fc::sha256 {
        using fc::sha256::sha256;

        template<typename T>
        inline friend T& operator<<( T& ds, const chain_id_type& cid ) {
            ds.write( cid.data(), cid.data_size() );
            return ds;
        }

        template<typename T>
        inline friend T& operator>>( T& ds, chain_id_type& cid ) {
            ds.read( cid.data(), cid.data_size() );
            return ds;
        }

        void reflector_init()const;

    public:
        chain_id_type() = default;

        // Some exceptions are unfortunately necessary:
        template<typename T>
        friend T fc::variant::as()const;
    };
}