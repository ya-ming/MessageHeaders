/**
 * @file MessageHeaders.cpp
 * 
 * This module contains the implementation of the MessageHeaders::MessageHeaders class.
 * 
 * 2019 by YaMing Wu
 */

#include <MessageHeaders/MessageHeaders.hpp>

namespace {
    /**
     * These are the characters that are considered white space
     * and should be stripped off by the Strip() functions.
     */
    constexpr const char* WHITESPACE = " \r\n\t";

    /**
     * This function returns a copy of the given string with any
     * whitespace at the beginning and end stripped off.
     *
     * @param[in] s
     *      This is the string to strip.
     *
     * @return
     *      The stripped string is returned.
     */
    std::string StripMarginWhitespace(const std::string& s) {
        const auto marginLeft = s.find_first_not_of(WHITESPACE);
        const auto marginRight = s.find_last_not_of(WHITESPACE);
        if (marginLeft == std::string::npos) {
            return "";
        }
        else {
            return s.substr(marginLeft, marginRight - marginLeft + 1);
        }
    }
}

namespace MessageHeaders {
    MessageHeaders::Header::Header(
        const HeaderName& newName,
        const HeaderValue& newValue
    ) : name(newName), value(newValue)
    {

    }

    /**
     * This contains the private properties of a MessageHeaders instance.
     */
    struct MessageHeaders::Impl {
        Headers headers;
        std::string body;
    };

    MessageHeaders::~MessageHeaders() = default;

    MessageHeaders::MessageHeaders() 
        : impl_(new Impl) {

    }

    bool MessageHeaders::ParseFromString(const std::string& rawMessage) {
        size_t offset = 0;
        while (offset < rawMessage.length()) {
            auto lineTerminator = rawMessage.find('\r\n', offset);
            // No line terminator
            if (lineTerminator == std::string::npos) {
                break;
            }
            if (lineTerminator == offset) {
                offset += 2;
                break;
            }

            auto nameValueDelimiter = rawMessage.find(':', offset);
            // No value for a specific name
            if (nameValueDelimiter == std::string::npos) {
                return false;
            }

            HeaderName name;
            HeaderValue value;
            name = rawMessage.substr(offset, nameValueDelimiter - offset);
            value = StripMarginWhitespace(rawMessage.substr(nameValueDelimiter + 1, lineTerminator - nameValueDelimiter - 1));
            impl_->headers.emplace_back(name, value);
            offset = lineTerminator + 2;
        }
        impl_->body = rawMessage.substr(offset);
        return true;
    }

    auto MessageHeaders::GetHeaders() const -> Headers {
        return impl_->headers;
    }

    bool MessageHeaders::HasHeader(const HeaderName& name) const {
        for (const auto& header : impl_->headers) {
            if (header.name == name) {
                return true;
            }
        }
        return false;
    }

    std::string MessageHeaders::GetBody() const {
        return impl_->body;
    }
}
