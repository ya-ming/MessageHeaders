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
    ASSERT_EQ(rawMessage, msg.GenerateRawHeaders());
}

TEST(MessageHeaderTests, HttpServerResponseMessage) {
    MessageHeaders::MessageHeaders msg;
    const std::string rawHeaders = (
        "Date: Mon, 27 Jul 2009 12:28:53 GMT\r\n"
        "Server: Apache\r\n"
        "Last-Modified: Wed, 22 Jul 2009 19:15:56 GMT\r\n"
        "ETag: \"34aa387-d-1568eb00\"\r\n"
        "Accept-Ranges: bytes\r\n"
        "Content-Length: 51\r\n"
        "Vary: Accept-Encoding\r\n"
        "Content-Type: text/plain\r\n"
        "\r\n"
    );
    const std::string rawMessage = (
        rawHeaders
        + "Hello World! My payload includes a tailing CRLF.\r\n"
        );

    size_t bodyOffset;
    ASSERT_TRUE(msg.ParseRawMessage(rawMessage, bodyOffset));
    ASSERT_EQ(rawHeaders.length(), bodyOffset);

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
    ASSERT_EQ(rawHeaders, msg.GenerateRawHeaders());
}

TEST(MessageHeaderTests, HeaderLineAlmostTooLong) {
    MessageHeaders::MessageHeaders headers;
    headers.SetLineLimit(1000);
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
    ASSERT_TRUE(headers.ParseRawMessage(rawMessage));
    ASSERT_EQ(longestPossiblePoggers, headers.GetHeaderValue(testHeaderName));
}

TEST(MessageHeaderTests, HeaderLineTooLong) {
    MessageHeaders::MessageHeaders headers;
    headers.SetLineLimit(1000);
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
    ASSERT_FALSE(headers.ParseRawMessage(rawMessage));
}

TEST(MessageHeadersTests, GetValueOfPresentHeader) {
    MessageHeaders::MessageHeaders headers;
    const std::string rawMessage = (
        "User-Agent: curl/7.16.3 libcurl/7.16.3 OpenSSL/0.9.7l zlib/1.2.3\r\n"
        "Host: www.example.com\r\n"
        "Accept-Language: en, mi\r\n"
        "\r\n"
        );
    ASSERT_TRUE(headers.ParseRawMessage(rawMessage));
    ASSERT_EQ("www.example.com", headers.GetHeaderValue("Host"));
}

TEST(MessageHeaderTests, GetValueOfMissingHeader) {
    MessageHeaders::MessageHeaders headers;

    const std::string rawMessage = (
        "User-Agent: curl/7.16.3 libcurl/7.16.3 OpenSSL/0.9.7l zlib/1.2.3\r\n"
        "Host: www.example.com\r\n"
        "Accept-Language: en, mi\r\n"
        "\r\n"
        );
    ASSERT_TRUE(headers.ParseRawMessage(rawMessage));
    ASSERT_EQ("FeelsBadMan", headers.GetHeaderValue("PePe"));
}

TEST(MessageHeaderTests, HeaderWithNonAsciiCharacterInName) {
    MessageHeaders::MessageHeaders headers;
    const std::string rawMessage = (
        "User-Agent: curl/7.16.3 libcurl/7.16.3 OpenSSL/0.9.7l zlib/1.2.3\r\n"
        "Host: www.example.com\r\n"
        "Feels Bad Man: LUL\r\n"
        "Accept-Language: en, mi\r\n"
        "\r\n"
        );
    ASSERT_FALSE(headers.ParseRawMessage(rawMessage));
}

TEST(MessageHeadersTests, HeaderLineOver1000CharactersAllowedByDefault) {
    MessageHeaders::MessageHeaders headers;
    const std::string testHeaderName("X-Poggers");
    const std::string testHeaderNameWithDelimiters = testHeaderName + ": ";
    const std::string valueForHeaderLineLongerThan1000Characters(999 - testHeaderNameWithDelimiters.length(), 'X');
    const std::string rawMessage = (
        "User-Agent: curl/7.16.3 libcurl/7.16.3 OpenSSL/0.9.7l zlib/1.2.3\r\n"
        "Host: www.example.com\r\n"
        + testHeaderNameWithDelimiters + valueForHeaderLineLongerThan1000Characters + "\r\n"
        + "Accept-Language: en, mi\r\n"
        "\r\n"
        );
    ASSERT_TRUE(headers.ParseRawMessage(rawMessage));
    ASSERT_EQ(valueForHeaderLineLongerThan1000Characters, headers.GetHeaderValue(testHeaderName));
}

TEST(MessageHeadersTests, SetHeaderAdd) {
    MessageHeaders::MessageHeaders msg;
    const std::string rawMessage = (
        "User-Agent: curl/7.16.3 libcurl/7.16.3 OpenSSL/0.9.7l zlib/1.2.3\r\n"
        "Host: www.example.com\r\n"
        "Accept-Language: en, mi\r\n"
        "\r\n"
        );
    ASSERT_TRUE(msg.ParseRawMessage(rawMessage));
    msg.SetHeader("X", "PogChamp");
    ASSERT_EQ(
        "User-Agent: curl/7.16.3 libcurl/7.16.3 OpenSSL/0.9.7l zlib/1.2.3\r\n"
        "Host: www.example.com\r\n"
        "Accept-Language: en, mi\r\n"
        "X: PogChamp\r\n"
        "\r\n",
        msg.GenerateRawHeaders()
    );
}

TEST(MessageHeadersTests, SetHeaderReplace) {
    MessageHeaders::MessageHeaders msg;
    const std::string rawMessage = (
        "User-Agent: curl/7.16.3 libcurl/7.16.3 OpenSSL/0.9.7l zlib/1.2.3\r\n"
        "Host: www.example.com\r\n"
        "Accept-Language: en, mi\r\n"
        "\r\n"
        );
    ASSERT_TRUE(msg.ParseRawMessage(rawMessage));
    msg.SetHeader("Host", "example.com");
    ASSERT_EQ(
        "User-Agent: curl/7.16.3 libcurl/7.16.3 OpenSSL/0.9.7l zlib/1.2.3\r\n"
        "Host: example.com\r\n"
        "Accept-Language: en, mi\r\n"
        "\r\n",
        msg.GenerateRawHeaders()
    );
}

TEST(MessageHeadersTests, HeaderValueUnfolding) {
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
