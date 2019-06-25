//
// Created by zwg on 19-6-21.
//
#pragma once

#include "action_receipt.hpp"
#include "action.hpp"
#include "types.hpp"
#include "block_timestamp.hpp"
#include <boost/container/flat_set.hpp>
#include <vector>
#include "block.hpp"

namespace eosio {
    namespace chain {
        struct account_delta {
            account_delta( const account_name& n, int64_t d):account(n),delta(d){}
            account_delta() = default;
            account_name account;
            int64_t delta = 0;
            friend bool operator<( const account_delta& lhs, const account_delta& rhs ) { return lhs.account < rhs.account; }
        };
        struct base_action_trace {
            explicit base_action_trace( const action_receipt& r ):receipt(r){}
            base_action_trace() = default;
            action_receipt       receipt;
            action               act;
            bool                 context_free = false;
            fc::microseconds     elapsed;
            std::string               console;
            transaction_id_type  trx_id; ///< the transaction that generated this action
            uint32_t             block_num = 0;
            block_timestamp_type block_time;
            fc::optional<block_id_type>     producer_block_id;
            boost::container::flat_set<account_delta>         account_ram_deltas;
            fc::optional<fc::exception>     except;
        };
        struct action_trace : public base_action_trace {
            using base_action_trace::base_action_trace;
            std::vector<action_trace> inline_traces;
        };
        struct transaction_trace;
        using transaction_trace_ptr = std::shared_ptr<transaction_trace>;
        struct transaction_trace {
            transaction_id_type                        id;
            uint32_t                                   block_num = 0;
            block_timestamp_type                       block_time;
            fc::optional<block_id_type>                producer_block_id;
            fc::optional<transaction_receipt_header>   receipt;
            fc::microseconds                           elapsed;
            uint64_t                                   net_usage = 0;
            bool                                       scheduled = false;
            std::vector<action_trace>                       action_traces; ///< disposable

            transaction_trace_ptr                      failed_dtrx_trace;
            fc::optional<fc::exception>                except;
            std::exception_ptr                         except_ptr;
        };
    }
}

FC_REFLECT( eosio::chain::account_delta,
        (account)(delta) )

FC_REFLECT( eosio::chain::base_action_trace,
        (receipt)(act)(context_free)(elapsed)(console)(trx_id)
                (block_num)(block_time)(producer_block_id)(account_ram_deltas)(except) )

FC_REFLECT_DERIVED( eosio::chain::action_trace,
        (eosio::chain::base_action_trace), (inline_traces) )

FC_REFLECT( eosio::chain::transaction_trace, (id)(block_num)(block_time)(producer_block_id)
        (receipt)(elapsed)(net_usage)(scheduled)
        (action_traces)(failed_dtrx_trace)(except) )