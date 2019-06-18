//
// Created by zwg on 19-6-12.
//

#ifndef NODEOS_TPS_PROTOCOL_HPP
#define NODEOS_TPS_PROTOCOL_HPP

#include <chrono>
#include <string>
#include <vector>
#include <set>
#include "types.hpp"

namespace eosio {
    static_assert(sizeof(std::chrono::system_clock::duration::rep) >= 8, "system_clock is expected to be at least 64 bits");
    typedef std::chrono::system_clock::duration::rep tstamp;

    struct chain_size_message {
        uint32_t                   last_irreversible_block_num = 0;
        block_id_type              last_irreversible_block_id;
        uint32_t                   head_num = 0;
        block_id_type              head_id;
    };

    struct handshake_message {
        uint16_t                   network_version = 0; ///< incremental value above a computed base
        chain_id_type              chain_id; ///< used to identify chain
        fc::sha256                 node_id; ///< used to identify peers and prevent self-connect
        public_key_type     key; ///< authentication key; may be a producer or peer key, or empty
        tstamp                     time;
        fc::sha256                 token; ///< digest of time to prove we own the private key of the key above
        signature_type      sig; ///< signature for the digest
        std::string                     p2p_address;
        uint32_t                   last_irreversible_block_num = 0;
        block_id_type              last_irreversible_block_id;
        uint32_t                   head_num = 0;
        block_id_type              head_id;
        std::string                     os;
        std::string                     agent;
        int16_t                    generation;
    };

    enum go_away_reason {
        no_reason, ///< no reason to go away
        self, ///< the connection is to itself
        duplicate, ///< the connection is redundant
        wrong_chain, ///< the peer's chain id doesn't match
        wrong_version, ///< the peer's network version doesn't match
        forked, ///< the peer's irreversible blocks are different
        unlinkable, ///< the peer sent a block we couldn't use
        bad_transaction, ///< the peer sent a transaction that failed verification
        validation, ///< the peer sent a block that failed validation
        benign_other, ///< reasons such as a timeout. not fatal but warrant resetting
        fatal_other, ///< a catch-all for errors we don't have discriminated
        authentication ///< peer failed authenicatio
    };

    constexpr auto reason_str( go_away_reason rsn ) {
        switch (rsn ) {
            case no_reason : return "no reason";
            case self : return "self connect";
            case duplicate : return "duplicate";
            case wrong_chain : return "wrong chain";
            case wrong_version : return "wrong version";
            case forked : return "chain is forked";
            case unlinkable : return "unlinkable block received";
            case bad_transaction : return "bad transaction";
            case validation : return "invalid block";
            case authentication : return "authentication failure";
            case fatal_other : return "some other failure";
            case benign_other : return "some other non-fatal condition";
            default : return "some crazy reason";
        }
    }

    struct go_away_message {
        go_away_message (go_away_reason r = no_reason) : reason(r), node_id() {}
        go_away_reason reason;
        fc::sha256 node_id; ///< for duplicate notification
    };

    struct time_message {
        tstamp  org;       //!< origin timestamp
        tstamp  rec;       //!< receive timestamp
        tstamp  xmt;       //!< transmit timestamp
        mutable tstamp  dst;       //!< destination timestamp
    };

    enum id_list_modes {
        none,
        catch_up,
        last_irr_catch_up,
        normal
    };

    constexpr auto modes_str( id_list_modes m ) {
        switch( m ) {
            case none : return "none";
            case catch_up : return "catch up";
            case last_irr_catch_up : return "last irreversible";
            case normal : return "normal";
            default: return "undefined mode";
        }
    }

    template<typename T>
    struct select_ids {
        select_ids () : mode(none),pending(0),ids() {}
        id_list_modes  mode;
        uint32_t       pending;
        std::vector<T>      ids;
        bool           empty () const { return (mode == none || ids.empty()); }
    };

    using ordered_txn_ids = select_ids<transaction_id_type>;
    using ordered_blk_ids = select_ids<block_id_type>;

    struct notice_message {
        notice_message () : known_trx(), known_blocks() {}
        ordered_txn_ids known_trx;
        ordered_blk_ids known_blocks;
    };

    struct request_message {
        request_message () : req_trx(), req_blocks() {}
        ordered_txn_ids req_trx;
        ordered_blk_ids req_blocks;
    };

    struct sync_request_message {
        uint32_t start_block;
        uint32_t end_block;
    };

    struct request_p2p_message{
        bool discoverable;
    };

    struct response_p2p_message{
        bool discoverable;
        std::string p2p_peer_list;
    };

    template<uint16_t IntervalMs, uint64_t EpochMs>
    class block_timestamp {
    public:
        explicit block_timestamp( uint32_t s=0 ) :slot(s){}

        block_timestamp(const fc::time_point& t) {
            set_time_point(t);
        }

        block_timestamp(const fc::time_point_sec& t) {
            set_time_point(t);
        }

        static block_timestamp maximum() { return block_timestamp( 0xffff ); }
        static block_timestamp min() { return block_timestamp(0); }

        block_timestamp next() const {
            //EOS_ASSERT( std::numeric_limits<uint32_t>::max() - slot >= 1, fc::overflow_exception, "block timestamp overflow" );
            auto result = block_timestamp(*this);
            result.slot += 1;
            return result;
        }

        fc::time_point to_time_point() const {
            return (fc::time_point)(*this);
        }

        operator fc::time_point() const {
            int64_t msec = slot * (int64_t)IntervalMs;
            msec += EpochMs;
            return fc::time_point(fc::milliseconds(msec));
        }

        void operator = (const fc::time_point& t ) {
            set_time_point(t);
        }

        bool   operator > ( const block_timestamp& t )const   { return slot >  t.slot; }
        bool   operator >=( const block_timestamp& t )const   { return slot >= t.slot; }
        bool   operator < ( const block_timestamp& t )const   { return slot <  t.slot; }
        bool   operator <=( const block_timestamp& t )const   { return slot <= t.slot; }
        bool   operator ==( const block_timestamp& t )const   { return slot == t.slot; }
        bool   operator !=( const block_timestamp& t )const   { return slot != t.slot; }
        uint32_t slot;

    private:
        void set_time_point(const fc::time_point& t) {
            auto micro_since_epoch = t.time_since_epoch();
            auto msec_since_epoch  = micro_since_epoch.count() / 1000;
            slot = ( msec_since_epoch - EpochMs ) / IntervalMs;
        }

        void set_time_point(const fc::time_point_sec& t) {
            uint64_t  sec_since_epoch = t.sec_since_epoch();
            slot = (sec_since_epoch * 1000 - EpochMs) / IntervalMs;
        }
    }; // block_timestamp
    typedef block_timestamp<500,946684800000ll> block_timestamp_type;

    struct name {
        uint64_t value = 0;
        bool empty()const { return 0 == value; }
        bool good()const  { return !empty();   }

        name( const char* str )   { set(str);           }
        name( const std::string& str ) { set( str.c_str() ); }

        void set( const char* str );

        template<typename T>
        name( T v ):value(v){}
        name(){}

        explicit operator std::string()const;

        std::string to_string() const { return std::string(*this); }

        name& operator=( uint64_t v ) {
            value = v;
            return *this;
        }

        name& operator=( const std::string& n ) {
            value = name(n).value;
            return *this;
        }
        name& operator=( const char* n ) {
            value = name(n).value;
            return *this;
        }

        friend std::ostream& operator << ( std::ostream& out, const name& n ) {
            return out << std::string(n);
        }

        friend bool operator < ( const name& a, const name& b ) { return a.value < b.value; }
        friend bool operator <= ( const name& a, const name& b ) { return a.value <= b.value; }
        friend bool operator > ( const name& a, const name& b ) { return a.value > b.value; }
        friend bool operator >=( const name& a, const name& b ) { return a.value >= b.value; }
        friend bool operator == ( const name& a, const name& b ) { return a.value == b.value; }

        friend bool operator == ( const name& a, uint64_t b ) { return a.value == b; }
        friend bool operator != ( const name& a, uint64_t b ) { return a.value != b; }

        friend bool operator != ( const name& a, const name& b ) { return a.value != b.value; }

        operator bool()const            { return value; }
        operator uint64_t()const        { return value; }
        operator unsigned __int128()const       { return value; }
    };
    using account_name     = name;
    using checksum256_type    = fc::sha256;

    struct producer_key {
        account_name      producer_name;
        public_key_type   block_signing_key;

        friend bool operator == ( const producer_key& lhs, const producer_key& rhs ) {
            return std::tie( lhs.producer_name, lhs.block_signing_key ) == std::tie( rhs.producer_name, rhs.block_signing_key );
        }
        friend bool operator != ( const producer_key& lhs, const producer_key& rhs ) {
            return std::tie( lhs.producer_name, lhs.block_signing_key ) != std::tie( rhs.producer_name, rhs.block_signing_key );
        }
    };

    struct producer_schedule_type {
        uint32_t                                       version = 0; ///< sequentially incrementing version number
        std::vector<producer_key>                           producers;

        public_key_type get_producer_key( account_name p )const {
            for( const auto& i : producers )
                if( i.producer_name == p )
                    return i.block_signing_key;
            return public_key_type();
        }
    };
    typedef std::vector<std::pair<uint16_t,std::vector<char>>> extensions_type;
    using digest_type         = fc::sha256;
    struct block_header {
        block_timestamp_type             timestamp;
        account_name                     producer;

        uint16_t                         confirmed = 1;

        block_id_type                    previous;

        checksum256_type                 transaction_mroot; /// mroot of cycles_summary
        checksum256_type                 action_mroot; /// mroot of all delivered action receipts

        uint32_t                          schedule_version = 0;
        fc::optional<producer_schedule_type>  new_producers;
        extensions_type                   header_extensions; // [0] : mroot of block extensions


        digest_type       digest()const;
        block_id_type     id() const;
        uint32_t          block_num() const { return num_from_id(previous) + 1; }
        static uint32_t   num_from_id(const block_id_type& id);
        void              set_block_extensions_mroot(digest_type& mroot);
    };

    struct signed_block_header : public block_header
    {
        signature_type    producer_signature;
    };

    struct transaction_receipt_header {
        enum status_enum {
            executed  = 0, ///< succeed, no error handler executed
            soft_fail = 1, ///< objectively failed (not executed), error handler executed
            hard_fail = 2, ///< objectively failed and error handler objectively failed thus no state change
            delayed   = 3, ///< transaction delayed/deferred/scheduled for future execution
            expired   = 4  ///< transaction expired and storage space refuned to user
        };

        transaction_receipt_header():status(hard_fail){}
        explicit transaction_receipt_header( status_enum s ):status(s){}

        friend inline bool operator ==( const transaction_receipt_header& lhs, const transaction_receipt_header& rhs ) {
            return std::tie(lhs.status, lhs.cpu_usage_us, lhs.net_usage_words) == std::tie(rhs.status, rhs.cpu_usage_us, rhs.net_usage_words);
        }

        fc::enum_type<uint8_t,status_enum>   status;
        uint32_t                             cpu_usage_us = 0; ///< total billed CPU usage (microseconds)
        fc::unsigned_int                     net_usage_words; ///<  total billed NET usage, so we can reconstruct resource state when skipping context free data... hard failures...
    };

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
    };
    using action_name      = name;
    using permission_name  = name;
    struct permission_level {
        account_name    actor;
        permission_name permission;
    };
    using bytes               = std::vector<char>;
    struct action {
        account_name               account;
        action_name                name;
        std::vector<permission_level>   authorization;
        bytes                      data;

        action(){}

        template<typename T, std::enable_if_t<std::is_base_of<bytes, T>::value, int> = 1>
        action( std::vector<permission_level> auth, const T& value ) {
            account     = T::get_account();
            name        = T::get_name();
            authorization = move(auth);
            data.assign(value.data(), value.data() + value.size());
        }

        template<typename T, std::enable_if_t<!std::is_base_of<bytes, T>::value, int> = 1>
        action( std::vector<permission_level> auth, const T& value ) {
            account     = T::get_account();
            name        = T::get_name();
            authorization = move(auth);
            data        = fc::raw::pack(value);
        }

        action( std::vector<permission_level> auth, account_name account, action_name name, const bytes& data )
                : account(account), name(name), authorization(move(auth)), data(data) {
        }
    };

    struct transaction : public transaction_header {
        std::vector<action>         context_free_actions;
        std::vector<action>         actions;
        extensions_type        transaction_extensions;

        transaction_id_type        id()const;
        digest_type                sig_digest( const chain_id_type& chain_id, const std::vector<bytes>& cfd = std::vector<bytes>() )const;
        fc::microseconds           get_signature_keys( const std::vector<signature_type>& signatures,
                                                       const chain_id_type& chain_id,
                                                       fc::time_point deadline,
                                                       const std::vector<bytes>& cfd,
                                                       fc::flat_set<public_key_type>& recovered_pub_keys,
                                                       bool allow_duplicate_keys = false) const;

        uint32_t total_actions()const { return context_free_actions.size() + actions.size(); }
        account_name first_authorizor()const {
            for( const auto& a : actions ) {
                for( const auto& u : a.authorization )
                    return u.actor;
            }
            return account_name();
        }

    };
    using private_key_type = fc::crypto::private_key;
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

        transaction_id_type id()const { return unpacked_trx.id(); }
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

    struct transaction_receipt : public transaction_receipt_header {

        transaction_receipt():transaction_receipt_header(){}
        explicit transaction_receipt( const transaction_id_type& tid ):transaction_receipt_header(executed),trx(tid){}
        explicit transaction_receipt( const packed_transaction& ptrx ):transaction_receipt_header(executed),trx(ptrx){}

        fc::static_variant<transaction_id_type, packed_transaction> trx;

        digest_type digest()const {
            digest_type::encoder enc;
            fc::raw::pack( enc, status );
            fc::raw::pack( enc, cpu_usage_us );
            fc::raw::pack( enc, net_usage_words );
            if( trx.contains<transaction_id_type>() )
                fc::raw::pack( enc, trx.get<transaction_id_type>() );
            else
                fc::raw::pack( enc, trx.get<packed_transaction>().packed_digest() );
            return enc.result();
        }
    };

    struct signed_block : public signed_block_header {
    private:
        signed_block( const signed_block& ) = default;
    public:
        signed_block() = default;
        explicit signed_block( const signed_block_header& h ):signed_block_header(h){}
        signed_block( signed_block&& ) = default;
        signed_block& operator=(const signed_block&) = delete;
        signed_block clone() const { return *this; }

        std::vector<transaction_receipt>   transactions; /// new or generated transactions
        extensions_type               block_extensions;
    };

    enum class pbft_message_type : uint16_t {
        prepare,
        commit,
        checkpoint,
        view_change,
        new_view
    };

    struct block_info_type {
        block_id_type   block_id;

        block_num_type  block_num() const {
            return fc::endian_reverse_u32(block_id._hash[0]);
        }

        bool operator==(const block_info_type &rhs) const {
            return block_id == rhs.block_id;
        }

        bool operator!=(const block_info_type &rhs) const {
            return !(*this == rhs);
        }
    };


    struct pbft_message_common {
        pbft_message_type   type;
        explicit pbft_message_common(pbft_message_type t): type{t} {};

        std::string              uuid;
        public_key_type     sender;
        chain_id_type       chain_id = chain_id_type("");
        fc::time_point          timestamp = fc::time_point::now();

        bool operator==(const pbft_message_common &rhs) const {
            return type == rhs.type
                   && chain_id == rhs.chain_id
                   && sender == rhs.sender;
        }

        bool empty() const {
            return uuid.empty()
                   && sender == public_key_type()
                   && chain_id == chain_id_type("");
        }

        ~pbft_message_common() = default;
    };

    struct pbft_prepare {
        explicit pbft_prepare() = default;

        pbft_message_common common = pbft_message_common(pbft_message_type::prepare);
        pbft_view_type      view = 0;
        block_info_type     block_info;
        signature_type      sender_signature;

        bool operator==(const pbft_prepare &rhs) const {
            return common == rhs.common
                   && view == rhs.view
                   && block_info == rhs.block_info;
        }

        bool operator<(const pbft_prepare &rhs) const {
            if (block_info.block_num() < rhs.block_info.block_num()) {
                return true;
            } else if (block_info.block_num() == rhs.block_info.block_num()) {
                return view < rhs.view;
            } else {
                return false;
            }
        }

        digest_type digest() const {
            digest_type::encoder enc;
            fc::raw::pack(enc, common);
            fc::raw::pack(enc, view);
            fc::raw::pack(enc, block_info);
            return enc.result();
        }

        bool is_signature_valid() const {
            try {
                auto pk = fc::crypto::public_key(sender_signature, digest(), true);
                return common.sender == pk;
            } catch (fc::exception & /*e*/) {
                return false;
            }
        }
    };

    struct pbft_commit {
        explicit pbft_commit() = default;

        pbft_message_common common = pbft_message_common(pbft_message_type::commit);
        pbft_view_type      view = 0;
        block_info_type     block_info;
        signature_type      sender_signature;

        bool operator==(const pbft_commit &rhs) const {
            return common == rhs.common
                   && view == rhs.view
                   && block_info == rhs.block_info;
        }

        bool operator<(const pbft_commit &rhs) const {
            if (block_info.block_num() < rhs.block_info.block_num()) {
                return true;
            } else if (block_info.block_num() == rhs.block_info.block_num()) {
                return view < rhs.view;
            } else {
                return false;
            }
        }

        digest_type digest() const {
            digest_type::encoder enc;
            fc::raw::pack(enc, common);
            fc::raw::pack(enc, view);
            fc::raw::pack(enc, block_info);
            return enc.result();
        }

        bool is_signature_valid() const {
            try {
                auto pk = fc::crypto::public_key(sender_signature, digest(), true);
                return common.sender == pk;
            } catch (fc::exception & /*e*/) {
                return false;
            }
        }
    };

    struct pbft_prepared_certificate {
        explicit pbft_prepared_certificate() = default;

        block_info_type      block_info;
        std::set<block_id_type>   pre_prepares;
        std::vector<pbft_prepare> prepares;

        bool operator<(const pbft_prepared_certificate &rhs) const {
            return block_info.block_num() < rhs.block_info.block_num();
        }

        bool empty() const {
            return block_info == block_info_type()
                   && prepares.empty();
        }
    };

    struct pbft_committed_certificate {
        explicit pbft_committed_certificate() = default;

        block_info_type     block_info;
        std::vector<pbft_commit> commits;

        bool operator<(const pbft_committed_certificate &rhs) const {
            return block_info.block_num() < rhs.block_info.block_num();
        }

        bool empty() const {
            return block_info == block_info_type()
                   && commits.empty();
        }
    };

    struct pbft_checkpoint {
        explicit pbft_checkpoint() = default;

        pbft_message_common common = pbft_message_common(pbft_message_type::checkpoint);
        block_info_type     block_info;
        signature_type      sender_signature;

        bool operator==(const pbft_checkpoint &rhs) const {
            return common == rhs.common
                   && block_info == rhs.block_info;
        }

        bool operator!=(const pbft_checkpoint &rhs) const {
            return !(*this == rhs);
        }

        bool operator<(const pbft_checkpoint &rhs) const {
            return block_info.block_num() < rhs.block_info.block_num();
        }

        digest_type digest() const {
            digest_type::encoder enc;
            fc::raw::pack(enc, common);
            fc::raw::pack(enc, block_info);
            return enc.result();
        }

        bool is_signature_valid() const {
            try {
                auto pk = fc::crypto::public_key(sender_signature, digest(), true);
                return common.sender == pk;
            } catch (fc::exception & /*e*/) {
                return false;
            }
        }
    };

    struct pbft_stable_checkpoint {
        explicit pbft_stable_checkpoint() = default;

        block_info_type         block_info;
        std::vector<pbft_checkpoint> checkpoints;

        bool operator<(const pbft_stable_checkpoint &rhs) const {
            return block_info.block_num() < rhs.block_info.block_num();
        }

        bool empty() const {
            return block_info == block_info_type()
                   && checkpoints.empty();
        }
    };

    struct pbft_view_change {
        explicit pbft_view_change() = default;

        pbft_message_common                 common = pbft_message_common(pbft_message_type::view_change);
        pbft_view_type                      current_view = 0;
        pbft_view_type                      target_view = 1;
        pbft_prepared_certificate           prepared_cert;
        std::vector<pbft_committed_certificate>  committed_cert;
        pbft_stable_checkpoint              stable_checkpoint;
        signature_type                      sender_signature;

        bool operator<(const pbft_view_change &rhs) const {
            return target_view < rhs.target_view;
        }

        digest_type digest() const {
            digest_type::encoder enc;
            fc::raw::pack(enc, common);
            fc::raw::pack(enc, current_view);
            fc::raw::pack(enc, target_view);
            fc::raw::pack(enc, prepared_cert);
            fc::raw::pack(enc, committed_cert);
            fc::raw::pack(enc, stable_checkpoint);
            return enc.result();
        }

        bool is_signature_valid() const {
            try {
                auto pk = fc::crypto::public_key(sender_signature, digest(), true);
                return common.sender == pk;
            } catch (fc::exception & /*e*/) {
                return false;
            }
        }

        bool empty() const {
            return common.empty()
                   && current_view == 0
                   && target_view == 0
                   && prepared_cert.empty()
                   && committed_cert.empty()
                   && stable_checkpoint.empty()
                   && sender_signature == signature_type();
        }
    };

    struct pbft_view_changed_certificate {
        explicit pbft_view_changed_certificate() = default;

        pbft_view_type              target_view = 0;
        std::vector<pbft_view_change>    view_changes;

        bool empty() const {
            return target_view == 0
                   && view_changes.empty();
        }
    };

    struct pbft_new_view {
        explicit pbft_new_view() = default;

        pbft_message_common                 common = pbft_message_common(pbft_message_type::new_view);
        pbft_view_type                      new_view = 0;
        pbft_prepared_certificate           prepared_cert;
        std::vector<pbft_committed_certificate>  committed_cert;
        pbft_stable_checkpoint              stable_checkpoint;
        pbft_view_changed_certificate       view_changed_cert;
        signature_type                      sender_signature;

        bool operator<(const pbft_new_view &rhs) const {
            return new_view < rhs.new_view;
        }

        digest_type digest() const {
            digest_type::encoder enc;
            fc::raw::pack(enc, common);
            fc::raw::pack(enc, new_view);
            fc::raw::pack(enc, prepared_cert);
            fc::raw::pack(enc, committed_cert);
            fc::raw::pack(enc, stable_checkpoint);
            fc::raw::pack(enc, view_changed_cert);
            return enc.result();
        }

        bool is_signature_valid() const {
            try {
                auto pk = fc::crypto::public_key(sender_signature, digest(), true);
                return common.sender == pk;
            } catch (fc::exception & /*e*/) {
                return false;
            }
        }

        bool empty() const {
            return common.empty()
                   && new_view == 0
                   && prepared_cert.empty()
                   && committed_cert.empty()
                   && stable_checkpoint.empty()
                   && view_changed_cert.empty()
                   && sender_signature == signature_type();
        }
    };

    struct checkpoint_request_message {
        uint32_t start_block;
        uint32_t end_block;
    };

    struct compressed_pbft_message {
        std::vector<char> content;
    };

    struct pbft_state {
        block_id_type block_id;
        block_num_type block_num = 0;
        std::vector<pbft_prepare> prepares;
        bool is_prepared = false;
        std::vector<pbft_commit> commits;
        bool is_committed = false;
    };

    struct pbft_view_change_state {
        pbft_view_type view;
        std::vector<pbft_view_change> view_changes;
        bool is_view_changed = false;
    };

    struct pbft_checkpoint_state {
        block_id_type block_id;
        block_num_type block_num = 0;
        std::vector<pbft_checkpoint> checkpoints;
        bool is_stable = false;
    };

    using net_message = fc::static_variant<handshake_message,
            chain_size_message,
            go_away_message,
            time_message,
            notice_message,
            request_message,
            sync_request_message,
            signed_block,       // which = 7
            packed_transaction, // which = 8
            response_p2p_message,
            request_p2p_message,
            pbft_prepare,
            pbft_commit,
            pbft_view_change,
            pbft_new_view,
            pbft_checkpoint,
            pbft_stable_checkpoint,
            checkpoint_request_message,
            compressed_pbft_message>;

} // namespace eosio

FC_REFLECT(eosio::block_header, (timestamp)(producer)(confirmed)(previous)(transaction_mroot)(action_mroot)(schedule_version)(new_producers)(header_extensions))
FC_REFLECT_DERIVED(eosio::signed_block_header, (eosio::block_header), (producer_signature))
FC_REFLECT(eosio::block_timestamp_type, (slot))
FC_REFLECT( eosio::name, (value) )
FC_REFLECT( eosio::producer_key, (producer_name)(block_signing_key) )
FC_REFLECT( eosio::producer_schedule_type, (version)(producers) )
FC_REFLECT_ENUM( eosio::transaction_receipt::status_enum,
                 (executed)(soft_fail)(hard_fail)(delayed)(expired) )
FC_REFLECT( eosio::transaction_header, (expiration)(ref_block_num)(ref_block_prefix)
        (max_net_usage_words)(max_cpu_usage_ms)(delay_sec) )
FC_REFLECT_DERIVED( eosio::transaction, (eosio::transaction_header), (context_free_actions)(actions)(transaction_extensions) )
FC_REFLECT_DERIVED( eosio::signed_transaction, (eosio::transaction), (signatures)(context_free_data) )
FC_REFLECT_ENUM( eosio::packed_transaction::compression_type, (none)(zlib))


FC_REFLECT(eosio::transaction_receipt_header, (status)(cpu_usage_us)(net_usage_words) )
FC_REFLECT_DERIVED(eosio::transaction_receipt, (eosio::transaction_receipt_header), (trx) )

FC_REFLECT( eosio::handshake_message, (network_version)(chain_id)(node_id)(key)(time)(token)(sig)(p2p_address)(last_irreversible_block_num)(last_irreversible_block_id)(head_num)(head_id)(os)(agent)(generation) )
FC_REFLECT( eosio::chain_size_message, (last_irreversible_block_num)(last_irreversible_block_id)(head_num)(head_id))
FC_REFLECT( eosio::go_away_message, (reason)(node_id) )
FC_REFLECT( eosio::time_message, (org)(rec)(xmt)(dst) )
FC_REFLECT( eosio::notice_message, (known_trx)(known_blocks) )
FC_REFLECT( eosio::request_message, (req_trx)(req_blocks) )
FC_REFLECT( eosio::sync_request_message, (start_block)(end_block) )
FC_REFLECT_DERIVED(eosio::signed_block, (eosio::signed_block_header), (transactions)(block_extensions) )
FC_REFLECT( eosio::packed_transaction, (signatures)(compression)(packed_context_free_data)(packed_trx) )
FC_REFLECT( eosio::request_p2p_message, (discoverable) )
FC_REFLECT( eosio::response_p2p_message, (discoverable)(p2p_peer_list) )
FC_REFLECT( eosio::select_ids<fc::sha256>, (mode)(pending)(ids) )
FC_REFLECT(eosio::block_info_type, (block_id))
FC_REFLECT_ENUM(eosio::pbft_message_type, (prepare)(commit)(checkpoint)(view_change)(new_view))
FC_REFLECT(eosio::pbft_message_common, (type)(uuid)(sender)(chain_id)(timestamp))
FC_REFLECT(eosio::pbft_prepare, (common)(view)(block_info)(sender_signature))
FC_REFLECT(eosio::pbft_commit, (common)(view)(block_info)(sender_signature))
FC_REFLECT(eosio::pbft_checkpoint,(common)(block_info)(sender_signature))
FC_REFLECT(eosio::pbft_view_change, (common)(current_view)(target_view)(prepared_cert)(committed_cert)(stable_checkpoint)(sender_signature))
FC_REFLECT(eosio::pbft_new_view, (common)(new_view)(prepared_cert)(committed_cert)(stable_checkpoint)(view_changed_cert)(sender_signature))
FC_REFLECT(eosio::pbft_prepared_certificate, (block_info)(pre_prepares)(prepares))
FC_REFLECT(eosio::pbft_committed_certificate,(block_info)(commits))
FC_REFLECT(eosio::pbft_view_changed_certificate, (target_view)(view_changes))
FC_REFLECT(eosio::pbft_stable_checkpoint, (block_info)(checkpoints))
FC_REFLECT(eosio::pbft_state, (block_id)(block_num)(prepares)(is_prepared)(commits)(is_committed))
FC_REFLECT(eosio::pbft_view_change_state, (view)(view_changes)(is_view_changed))
FC_REFLECT(eosio::pbft_checkpoint_state, (block_id)(block_num)(checkpoints)(is_stable))
FC_REFLECT( eosio::checkpoint_request_message, (start_block)(end_block) )
FC_REFLECT( eosio::compressed_pbft_message, (content))

#endif //NODEOS_TPS_PROTOCOL_HPP
