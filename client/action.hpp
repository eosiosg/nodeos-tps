//
// Created by zwg on 19-6-20.
//
/**
 *  @file
 *  @copyright defined in eos/LICENSE
 */
#pragma once

#include "types.hpp"
#include "exceptions.hpp"

namespace eosio { namespace chain {

        struct permission_level {
            account_name    actor;
            permission_name permission;
        };

        inline bool operator== (const permission_level& lhs, const permission_level& rhs) {
            return std::tie(lhs.actor, lhs.permission) == std::tie(rhs.actor, rhs.permission);
        }

        inline bool operator!= (const permission_level& lhs, const permission_level& rhs) {
            return std::tie(lhs.actor, lhs.permission) != std::tie(rhs.actor, rhs.permission);
        }

        inline bool operator< (const permission_level& lhs, const permission_level& rhs) {
            return std::tie(lhs.actor, lhs.permission) < std::tie(rhs.actor, rhs.permission);
        }

        inline bool operator<= (const permission_level& lhs, const permission_level& rhs) {
            return std::tie(lhs.actor, lhs.permission) <= std::tie(rhs.actor, rhs.permission);
        }

        inline bool operator> (const permission_level& lhs, const permission_level& rhs) {
            return std::tie(lhs.actor, lhs.permission) > std::tie(rhs.actor, rhs.permission);
        }

        inline bool operator>= (const permission_level& lhs, const permission_level& rhs) {
            return std::tie(lhs.actor, lhs.permission) >= std::tie(rhs.actor, rhs.permission);
        }

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

        struct action_notice : public action {
            account_name receiver;
        };

    } } /// namespace eosio::chain

FC_REFLECT( eosio::chain::permission_level, (actor)(permission) )
FC_REFLECT( eosio::chain::action, (account)(name)(authorization)(data) )
