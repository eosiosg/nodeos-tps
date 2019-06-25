//
// Created by zwg on 19-6-20.
//

#pragma once

#include <fc/io/enum_type.hpp>
#include <tuple>

namespace eosio {
    namespace chain {
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
    }
}