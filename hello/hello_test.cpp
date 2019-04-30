//
// Created by deadlock on 2019-04-30.
//

#include <gtest/gtest.h>
#include "hello.hpp"

using namespace hello;

TEST/*NOLINT*/(hello, can_build) {
    EXPECT_TRUE(true);
}

TEST/*NOLINT*/(hello, can_say_good_morning) {
    std::string to_whom = "alice";
    auto greeting = say_good_morning(to_whom);
    auto expected_greeting = "good morning alice";

    EXPECT_EQ(greeting, expected_greeting);
}