//
// Created by deadlock on 2019-04-30.
//

#include "hello.hpp"
#include <string>

namespace hello {

std::string say_good_morning(std::string& to_whom)
{
    return "good morning "+to_whom;
}

}