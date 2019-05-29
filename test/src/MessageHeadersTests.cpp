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
    const std::string rawMessage = (
        "User-Agent: curl/7.16.3 libcurl/7.16.3 OpenSSL/0.9.7l zlib/1.2.3\r\n"
        "Host: www.example.com\r\n"
        "Accept-Language: en, mi\r\n"
        "\r\n"
    );

    ASSERT_TRUE(msg.ParseRawMessage(rawMessage));

    const auto headers = msg.GetAll();
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
    ASSERT_EQ(rawMessage, msg.GenerateRawMessage());
}

TEST(MessageHeaderTests, HttpServerResponseMessage) {
    MessageHeaders::MessageHeaders msg;
    const std::string rawMessage = (
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
    );

    ASSERT_TRUE(msg.ParseRawMessage(rawMessage));

    const auto headers = msg.GetAll();
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
    ASSERT_EQ(rawMessage, msg.GenerateRawMessage());
}

TEST(MessageHeaderTests, HeaderLineAlmostTooLong) {
    MessageHeaders::MessageHeaders msg;

    const std::string testHeaderName("X-Poggers");
    const std::string testHeaderNameWithDelimiters = testHeaderName + ": ";
    const std::string longestPossiblePoggers(998 - testHeaderNameWithDelimiters.length(), 'X');
    const std::string rawMessage = (
        "User-Agent: curl/7.16.3 libcurl/7.16.3 OpenSSL/0.9.7l zlib/1.2.3\r\n"
        "Host: www.example.com\r\n"
        + testHeaderNameWithDelimiters + longestPossiblePoggers + "\r\n"
        + "Accept-Language: en, mi\r\n"
        "\r\n"
        );
    ASSERT_TRUE(msg.ParseRawMessage(rawMessage));
    ASSERT_EQ(longestPossiblePoggers, msg.GetHeaderValue(testHeaderName));
}

TEST(MessageHeaderTests, HeaderLineTooLong) {
    MessageHeaders::MessageHeaders msg;

    const std::string testHeaderName("X-Poggers");
    const std::string testHeaderNameWithDelimiters = testHeaderName + ": ";
    const std::string longestPossiblePoggers(999 - testHeaderNameWithDelimiters.length(), 'X');
    const std::string rawMessage = (
        "User-Agent: curl/7.16.3 libcurl/7.16.3 OpenSSL/0.9.7l zlib/1.2.3\r\n"
        "Host: www.example.com\r\n"
        + testHeaderNameWithDelimiters + longestPossiblePoggers + "\r\n"
        + "Accept-Language: en, mi\r\n"
        "\r\n"
        );
    ASSERT_FALSE(msg.ParseRawMessage(rawMessage));
}

TEST(MessageHeaderTests, GetValueOfMissingHeader) {
    MessageHeaders::MessageHeaders msg;

    const std::string rawMessage = (
        "User-Agent: curl/7.16.3 libcurl/7.16.3 OpenSSL/0.9.7l zlib/1.2.3\r\n"
        "Host: www.example.com\r\n"
        "Accept-Language: en, mi\r\n"
        "\r\n"
        );
    ASSERT_TRUE(msg.ParseRawMessage(rawMessage));
    ASSERT_EQ("FeelsBadMan", msg.GetHeaderValue("PePe"));
}

TEST(MessageHeaderTests, HeaderWithNonAsciiCharacterInName) {
    MessageHeaders::MessageHeaders msg;
    const std::string rawMessage = (
        "User-Agent: curl/7.16.3 libcurl/7.16.3 OpenSSL/0.9.7l zlib/1.2.3\r\n"
        "Host: www.example.com\r\n"
        "Feels Bad Man: LUL\r\n"
        "Accept-Language: en, mi\r\n"
        "\r\n"
        );
    ASSERT_FALSE(msg.ParseRawMessage(rawMessage));
}

TEST(MessageHeaderTests, HeaderWithLoneCRInMiddle) {
    MessageHeaders::MessageHeaders msg;
    const std::string rawMessage = (
        "User-Agent: curl/7.16.3 libcurl/7.16.3 OpenSSL/0.9.7l zlib/1.2.3\r\n"
        "Host: www.example.com\r\n"
        "\r\n"
        "admiralB\radmiralEmo"
        );
    ASSERT_FALSE(msg.ParseRawMessage(rawMessage));
}

TEST(MessageHeaderTests, HeaderWithLoneCRAtEnd) {
    MessageHeaders::MessageHeaders msg;
    const std::string rawMessage = (
        "User-Agent: curl/7.16.3 libcurl/7.16.3 OpenSSL/0.9.7l zlib/1.2.3\r\n"
        "Host: www.example.com\r\n"
        "\r\n"
        "admiralBadmiralEmo\r"
        );
    ASSERT_FALSE(msg.ParseRawMessage(rawMessage));
}

TEST(MessageHeaderTests, HeaderWithLoneLFInMiddle) {
    MessageHeaders::MessageHeaders msg;
    const std::string rawMessage = (
        "User-Agent: curl/7.16.3 libcurl/7.16.3 OpenSSL/0.9.7l zlib/1.2.3\r\n"
        "Host: www.example.com\r\n"
        "\r\n"
        "admiralB\nadmiralEmo"
        );
    ASSERT_FALSE(msg.ParseRawMessage(rawMessage));
}

TEST(MessageHeaderTests, HeaderWithLoneLFAtEnd) {
    MessageHeaders::MessageHeaders msg;
    const std::string rawMessage = (
        "User-Agent: curl/7.16.3 libcurl/7.16.3 OpenSSL/0.9.7l zlib/1.2.3\r\n"
        "Host: www.example.com\r\n"
        "\r\n"
        "admiralBadmiralEmo\n"
        );
    ASSERT_FALSE(msg.ParseRawMessage(rawMessage));
}

TEST(MessageHeadersTests, FoldedHeaderValue) {
    MessageHeaders::MessageHeaders msg;
    const std::string rawMessage = (
        "User-Agent: curl/7.16.3 libcurl/7.16.3 OpenSSL/0.9.7l zlib/1.2.3\r\n"
        "Host: www.example.com\r\n"
        "Accept-Language: en, mi\r\n"
        "Subject: This\r\n"
        " is a test\r\n"
        "\r\n"
        );
    ASSERT_TRUE(msg.ParseRawMessage(rawMessage));
    ASSERT_EQ("This is a test", msg.GetHeaderValue("Subject"));
}
