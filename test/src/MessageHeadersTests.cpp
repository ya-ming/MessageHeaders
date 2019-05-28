/**
 * @file MessageHeaderTests.cpp
 *
 * This module contains the unit tests of the
 * MessageHeaders::MessageHeaders class.
 *
 * 2019 by YaMing Wu
 */

#include <gtest/gtest.h>
#include <MessageHeaders/MessageHeaders.hpp>

 TEST(MessageHeaderTests, HttpClientRequestMessage) {
    MessageHeaders::MessageHeaders msg;
    ASSERT_TRUE(
        msg.ParseFromString(
            "User-Agent: curl/7.16.3 libcurl/7.16.3 OpenSSL/0.9.7l zlib/1.2.3\r\n"
            "Host: www.example.com\r\n"
            "Accept-Language: en, mi\r\n"
            "\r\n"
        )
    );

    const auto headers = msg.GetHeaders();
    struct ExpectedHeader {
        std::string name;
        std::string value;
    };

    const std::vector<ExpectedHeader> expectedHeaders{
        {"User-Agent", "curl/7.16.3 libcurl/7.16.3 OpenSSL/0.9.7l zlib/1.2.3"},
        {"Host", "www.example.com"},
        {"Accept-Language", "en, mi"},
    };

    ASSERT_EQ(expectedHeaders.size(), headers.size());
    for (size_t i = 0; i < expectedHeaders.size(); ++i) {
        ASSERT_EQ(expectedHeaders[i].name, headers[i].name);
        ASSERT_EQ(expectedHeaders[i].value, headers[i].value);
    }

    ASSERT_TRUE(msg.HasHeader("Host"));
    ASSERT_FALSE(msg.HasHeader("Foobar"));
    ASSERT_EQ("", msg.GetBody());
}

TEST(MessageHeaderTests, HttpServerResponseMessage) {
    MessageHeaders::MessageHeaders msg;
    ASSERT_TRUE(
        msg.ParseFromString(
            "Date: Mon, 27 Jul 2009 12:28:53 GMT\r\n"
            "Server: Apache\r\n"
            "Last-Modified: Wed, 22 Jul 2009 19:15:56 GMT\r\n"
            "ETag: \"34aa387-d-1568eb00\"\r\n"
            "Accept-Ranges: bytes\r\n"
            "Content-Length: 51\r\n"
            "Vary: Accept-Encoding\r\n"
            "Content-Type: text/plain\r\n"
            "\r\n"
            "Hello World! My payload includes a trailing CRLF.\r\n"
        )
    );
    const auto headers = msg.GetHeaders();
    struct ExpectedHeader {
        std::string name;
        std::string value;
    };
    const std::vector< ExpectedHeader > expectedHeaders{
        {"Date", "Mon, 27 Jul 2009 12:28:53 GMT"},
        {"Server", "Apache"},
        {"Last-Modified", "Wed, 22 Jul 2009 19:15:56 GMT"},
        {"ETag", "\"34aa387-d-1568eb00\""},
        {"Accept-Ranges", "bytes"},
        {"Content-Length", "51"},
        {"Vary", "Accept-Encoding"},
        {"Content-Type", "text/plain"},
    };
    ASSERT_EQ(expectedHeaders.size(), headers.size());
    for (size_t i = 0; i < expectedHeaders.size(); ++i) {
        ASSERT_EQ(expectedHeaders[i].name, headers[i].name);
        ASSERT_EQ(expectedHeaders[i].value, headers[i].value);
    }
    ASSERT_TRUE(msg.HasHeader("Last-Modified"));
    ASSERT_FALSE(msg.HasHeader("Foobar"));
    ASSERT_EQ("Hello World! My payload includes a trailing CRLF.\r\n", msg.GetBody());
}
