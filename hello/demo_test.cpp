//
// Created by deadlock on 2019-06-08.
//

#include <gtest/gtest.h>
#include "demo.hpp"

using namespace demo;

TEST/*NOLINT*/(text_serializer, can_build)
{
    EXPECT_TRUE(true);
}

TEST/*NOLINT*/(text_serializer, can_serialize_article)
{
    article obj{
            .page_num = 10,
            .title = "I am Mr title",
            .authors = {"alice", "bob", "charles"},
            .content = "content_is_very_long",
    };
    std::string expected_result = "1013I am Mr title35alice3bob7charles20content_is_very_long";
    text_serializer serializer;

    serializer.serialize(obj);

    EXPECT_EQ(serializer.get_result(), expected_result);
}

TEST/*NOLINT*/(text_serializer, can_serialize_reference)
{
    reference obj{
            .name = "GCPR",
            .link = "www.GCPR.org",
            .property = 'r',

    };
    std::string expected_result = "4GCPR12www.GCPR.orgr";
    text_serializer serializer;

    serializer.serialize(obj);

    EXPECT_EQ(serializer.get_result(), expected_result);
}

TEST/*NOLINT*/(text_serializer, can_serialize_summary)
{
    reference obj_ref{
            .name = "GCPR",
            .link = "www.GCPR.org",
            .property = 'r',

    };
    summary obj{
            .title = "I am Mr title",
            .content = "content_is_very_long",
            .article_reference = std::optional<reference>{obj_ref},
    };
    std::string expected_result = "13I am Mr title20content_is_very_long14GCPR12www.GCPR.orgr";
    text_serializer serializer;

    serializer.serialize(obj);

    EXPECT_EQ(serializer.get_result(), expected_result);
}

