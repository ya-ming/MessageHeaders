/**
 * @file MessageHeaders.cpp
 * 
 * This module contains the implementation of the MessageHeaders::MessageHeaders class.
 * 
 * 2019 by YaMing Wu
 */

#include <MessageHeaders/MessageHeaders.hpp>
#include <sstream>

namespace {
    /**
     * These are the characters that are considered white space
     * and should be stripped off by the Strip() functions.
     */
    const std::string WSP = " \t";

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
        const auto marginLeft = s.find_first_not_of(WSP);
        const auto marginRight = s.find_last_not_of(WSP);
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
        size_t lineLengthLimit = 0;
    };

    MessageHeaders::~MessageHeaders() = default;

    MessageHeaders::MessageHeaders() 
        : impl_(new Impl) {
    }

    void MessageHeaders::SetLineLimit(size_t newLineLengthLimit) {
        impl_->lineLengthLimit = newLineLengthLimit;
    }

    bool MessageHeaders::ParseRawMessage(const std::string& rawMessage, size_t& bodyOffset) {
        size_t offset = 0;
        while (offset < rawMessage.length()) {
            auto lineTerminator = rawMessage.find('\r\n', offset);
            // No line terminator
            if (lineTerminator == std::string::npos) {
                break;
            }

            if (impl_->lineLengthLimit > 0) {
                if (lineTerminator - offset + 2> impl_->lineLengthLimit) {
                    return false;
                }
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

            // Make sure all the header characters are printable
            for (auto c : name) {
                if (c < 33 || c > 126) {
                    return false;
                }
            }

            value = rawMessage.substr(nameValueDelimiter + 1, lineTerminator - nameValueDelimiter - 1);
            offset = lineTerminator + 2;

            for (;;) {
                const auto nextLineStart = lineTerminator + 2;
                auto nextLineTerminator = rawMessage.find("\r\n", nextLineStart);
                if (nextLineTerminator == std::string::npos) {
                    break;
                }
                const auto nextLineLength = nextLineTerminator - nextLineStart;
                if (
                    (nextLineLength > 2)
                    && (WSP.find(rawMessage[nextLineStart]) != std::string::npos)
                    ) {
                    value += rawMessage.substr(nextLineStart, nextLineLength);
                    offset = nextLineTerminator + 2;
                    lineTerminator = nextLineTerminator;
                }
                else {
                    break;
                }
            }
            value = StripMarginWhitespace(value);
            impl_->headers.emplace_back(name, value);
        }
        bodyOffset = offset;
        return true;
    }

    bool MessageHeaders::ParseRawMessage(const std::string& rawMessage) {
        size_t bodyOffset;
        return ParseRawMessage(rawMessage, bodyOffset);
    }

    std::string MessageHeaders::GenerateRawHeaders() const {
        std::ostringstream rawMessage;
        for (const auto& header : impl_->headers) {
            rawMessage << header.name << ": " << header.value << "\r\n";
        }

        rawMessage << "\r\n";
        return rawMessage.str();
    }

    auto MessageHeaders::GetAll() const -> Headers {
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

    auto MessageHeaders::GetHeaderValue(const HeaderName& name) const -> HeaderValue {
        for (const auto& header : impl_->headers) {
            if (header.name == name) {
                return header.value;
            }
        }
        return "FeelsBadMan";
    }
}
