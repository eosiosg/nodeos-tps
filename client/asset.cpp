//
// Created by zwg on 19-6-21.
//

/**
 *  @file
 *  @copyright defined in eos/LICENSE
 */
#include "asset.hpp"
#include <boost/rational.hpp>
#include <fc/reflect/variant.hpp>

namespace eosio {
    namespace chain {

        uint8_t asset::decimals()const {
            return sym.decimals();
        }

        std::string asset::symbol_name()const {
            return sym.name();
        }

        int64_t asset::precision()const {
            return sym.precision();
        }

        std::string asset::to_string()const {
            std::string sign = amount < 0 ? "-" : "";
            int64_t abs_amount = std::abs(amount);
            std::string result = fc::to_string( static_cast<int64_t>(abs_amount) / precision());
            if( decimals() )
            {
                auto fract = static_cast<int64_t>(abs_amount) % precision();
                result += "." + fc::to_string(precision() + fract).erase(0,1);
            }
            return sign + result + " " + symbol_name();
        }

        asset asset::from_string(const std::string& from)
        {
            try {
                std::string s = fc::trim(from);

                // Find space in order to split amount and symbol
                auto space_pos = s.find(' ');
                EOS_ASSERT((space_pos != std::string::npos), asset_type_exception, "Asset's amount and symbol should be separated with space");
                auto symbol_str = fc::trim(s.substr(space_pos + 1));
                auto amount_str = s.substr(0, space_pos);

                // Ensure that if decimal point is used (.), decimal fraction is specified
                auto dot_pos = amount_str.find('.');
                if (dot_pos != std::string::npos) {
                    EOS_ASSERT((dot_pos != amount_str.size() - 1), asset_type_exception, "Missing decimal fraction after decimal point");
                }

                // Parse symbol
                std::string precision_digit_str;
                if (dot_pos != std::string::npos) {
                    precision_digit_str = std::to_string(amount_str.size() - dot_pos - 1);
                } else {
                    precision_digit_str = "0";
                }

                std::string symbol_part = precision_digit_str + ',' + symbol_str;
                symbol sym = symbol::from_string(symbol_part);

                // Parse amount
                fc::safe<int64_t> int_part, fract_part;
                if (dot_pos != std::string::npos) {
                    int_part = fc::to_int64(amount_str.substr(0, dot_pos));
                    fract_part = fc::to_int64(amount_str.substr(dot_pos + 1));
                    if (amount_str[0] == '-') fract_part *= -1;
                } else {
                    int_part = fc::to_int64(amount_str);
                }

                fc::safe<int64_t> amount = int_part;
                amount *= fc::safe<int64_t>(sym.precision());
                amount += fract_part;

                return asset(amount.value, sym);
            }
            FC_CAPTURE_LOG_AND_RETHROW( (from) )
        }

    } }  // eosio::types
