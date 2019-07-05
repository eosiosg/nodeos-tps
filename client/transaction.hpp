//
// Created by zwg on 19-6-21.
//

#pragma once

#include "types.hpp"
#include "action.hpp"
#include <vector>

namespace eosio {
    namespace chain {
        typedef std::vector<std::pair<uint16_t,std::vector<char>>> extensions_type;
        struct transaction_header {
            fc::time_point_sec         expiration;   ///< the time at which a transaction expires
            uint16_t               ref_block_num       = 0U; ///< specifies a block num in the last 2^16 blocks.
            uint32_t               ref_block_prefix    = 0UL; ///< specifies the lower 32 bits of the blockid at get_ref_blocknum
            fc::unsigned_int       max_net_usage_words = 0UL; /// upper limit on total network bandwidth (in 8 byte words) billed for this transaction
            uint8_t                max_cpu_usage_ms    = 0; /// upper limit on the total CPU time billed for this transaction
            fc::unsigned_int       delay_sec           = 0UL; /// number of seconds to delay this transaction for during which it may be canceled.

            /**
             * @return the absolute block number given the relative ref_block_num
             */
            block_num_type get_ref_blocknum( block_num_type head_blocknum )const {
                return ((head_blocknum/0xffff)*0xffff) + head_blocknum%0xffff;
            }
            void set_reference_block( const block_id_type& reference_block );
            bool verify_reference_block( const block_id_type& reference_block )const;
            void validate()const;

            friend std::ostream& operator<<(std::ostream& output, const transaction_header& msg) {
                output << "expiration:" << msg.expiration.sec_since_epoch()
                       << ", ref_block_num:" << msg.ref_block_num
                       << ", ref_block_prefix:" << msg.ref_block_prefix
                       << ", max_net_usage_words:" << msg.max_net_usage_words
                       << ", max_cpu_usage_ms:" << uint32_t(msg.max_cpu_usage_ms)
                       << ", delay_sec:" << msg.delay_sec;
                return output;
            }
        };

        struct transaction : public transaction_header {
            std::vector<action>         context_free_actions;
            std::vector<action>         actions;
            extensions_type        transaction_extensions;
            transaction_id_type        id()const;
            friend std::ostream& operator<<(std::ostream& output, const transaction& msg) {
                output << static_cast<const transaction_header&>(msg);
                return output;
            }
            digest_type sig_digest( const chain_id_type& chain_id, const std::vector<bytes>& cfd )const;
            fc::microseconds get_signature_keys( const std::vector<signature_type>& signatures,
                    const chain_id_type& chain_id, fc::time_point deadline, const std::vector<bytes>& cfd,
                    fc::flat_set<public_key_type>& recovered_pub_keys, bool allow_duplicate_keys)const;
        };

        struct signed_transaction : public transaction
        {
            signed_transaction() = default;
//      signed_transaction( const signed_transaction& ) = default;
//      signed_transaction( signed_transaction&& ) = default;
            signed_transaction( transaction&& trx, const std::vector<signature_type>& signatures, const std::vector<bytes>& context_free_data)
                    : transaction(std::move(trx))
                    , signatures(signatures)
                    , context_free_data(context_free_data)
            {}
            signed_transaction( transaction&& trx, const std::vector<signature_type>& signatures, std::vector<bytes>&& context_free_data)
                    : transaction(std::move(trx))
                    , signatures(signatures)
                    , context_free_data(std::move(context_free_data))
            {}

            std::vector<signature_type>    signatures;
            std::vector<bytes>             context_free_data; ///< for each context-free action, there is an entry here

            const signature_type&     sign(const private_key_type& key, const chain_id_type& chain_id);
            signature_type            sign(const private_key_type& key, const chain_id_type& chain_id)const;
            fc::microseconds          get_signature_keys( const chain_id_type& chain_id, fc::time_point deadline,
                    fc::flat_set<public_key_type>& recovered_pub_keys,
                    bool allow_duplicate_keys = false )const;
        };

        struct packed_transaction {

            friend std::ostream& operator<<(std::ostream& output, const packed_transaction& msg) {
                output << msg.unpacked_trx;
                return output;
            }

            enum compression_type {
                none = 0,
                zlib = 1,
            };

            packed_transaction() = default;
            packed_transaction(packed_transaction&&) = default;
            explicit packed_transaction(const packed_transaction&) = default;
            packed_transaction& operator=(const packed_transaction&) = delete;
            packed_transaction& operator=(packed_transaction&&) = default;

            explicit packed_transaction(const signed_transaction& t, compression_type _compression = none)
                    :signatures(t.signatures), compression(_compression), unpacked_trx(t)
            {
                local_pack_transaction();
                local_pack_context_free_data();
            }

            explicit packed_transaction(signed_transaction&& t, compression_type _compression = none)
                    :signatures(t.signatures), compression(_compression), unpacked_trx(std::move(t))
            {
                local_pack_transaction();
                local_pack_context_free_data();
            }

            // used by abi_serializer
            packed_transaction( bytes&& packed_txn, std::vector<signature_type>&& sigs, bytes&& packed_cfd, compression_type _compression );
            packed_transaction( bytes&& packed_txn, std::vector<signature_type>&& sigs, std::vector<bytes>&& cfd, compression_type _compression );
            packed_transaction( transaction&& t, std::vector<signature_type>&& sigs, bytes&& packed_cfd, compression_type _compression );

            uint32_t get_unprunable_size()const;
            uint32_t get_prunable_size()const;

            digest_type packed_digest()const;

            bytes               get_raw_transaction()const;

            fc::time_point_sec                expiration()const { return unpacked_trx.expiration; }
            const std::vector<bytes>&          get_context_free_data()const { return unpacked_trx.context_free_data; }
            const transaction&            get_transaction()const { return unpacked_trx; }
            const signed_transaction&     get_signed_transaction()const { return unpacked_trx; }
            const std::vector<signature_type>& get_signatures()const { return signatures; }
            const fc::enum_type<uint8_t,compression_type>& get_compression()const { return compression; }
            const bytes&                  get_packed_context_free_data()const { return packed_context_free_data; }
            const bytes&                  get_packed_transaction()const { return packed_trx; }

        private:
            void local_unpack_transaction(std::vector<bytes>&& context_free_data);
            void local_unpack_context_free_data();
            void local_pack_transaction();
            void local_pack_context_free_data();

            friend struct fc::reflector<packed_transaction>;
            friend struct fc::reflector_init_visitor<packed_transaction>;
            void reflector_init();
        private:
            std::vector<signature_type>                  signatures;
            fc::enum_type<uint8_t,compression_type> compression;
            bytes                                   packed_context_free_data;
            bytes                                   packed_trx;

        private:
            // cache unpacked trx, for thread safety do not modify after construction
            signed_transaction                      unpacked_trx;
        };

    }
}


FC_REFLECT( eosio::chain::transaction_header, (expiration)(ref_block_num)(ref_block_prefix)
        (max_net_usage_words)(max_cpu_usage_ms)(delay_sec) )
FC_REFLECT_DERIVED( eosio::chain::transaction, (eosio::chain::transaction_header), (context_free_actions)(actions)(transaction_extensions) )
FC_REFLECT_DERIVED( eosio::chain::signed_transaction, (eosio::chain::transaction), (signatures)(context_free_data) )
FC_REFLECT_ENUM( eosio::chain::packed_transaction::compression_type, (none)(zlib))
FC_REFLECT( eosio::chain::packed_transaction, (signatures)(compression)(packed_context_free_data)(packed_trx) )
//FC_REFLECT_DERIVED( eosio::chain::deferred_transaction, (eosio::chain::signed_transaction), (sender_id)(sender)(payer)(execute_after) )
//FC_REFLECT( eosio::chain::deferred_reference, (sender)(sender_id) )
