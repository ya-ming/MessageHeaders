/**
 * @file MessageHeaders.cpp
 * 
 * This module contains the implementation of the MessageHeaders::MessageHeaders class.
 * 
 * 2019 by YaMing Wu
 */

#include <functional>
#include <MessageHeaders/MessageHeaders.hpp>
#include <sstream>

namespace {
    /**
     * These are the characters that are considered white space
     * and should be stripped off by the Strip() functions.
     */
    const std::string WSP = " \t";

    /**
     * This is the required line terminator for internet message header lines.
     */
    const std::string CRLF = "\r\n";

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

    /**
     * This is the type of function that is used as the strategy to
     * determine where to break a long string into two smaller strings.
     *
     * @param[in] s
     *     This is the string which we are considering breaking.
     *
     * @param[in] offset
     *     This is the offset into the given string of the part
     *     that is under consideration for breaking.
     *
     * @return
     *     The offset in the string where the string should be
     *     divided in two is returned.
     *
     * @note
     *     The given offset is returned if the string cannot be broken.
     */
    typedef std::function<size_t(const std::string& s, size_t offset)> StringFoldingStrategy;

    /**
     * This method breaks up the given input line into multiple output lines,
     * as needed, to ensure that no output line is longer than the given
     * line limit, including line terminators.
     *
     * @param[in] line
     *     This is the line to fold if necessary.
     *
     * @param[in] terminator
     *     This is the character sequence that separates lines.
     *
     * @param[in] limit
     *     This is the maximum number of characters allowed per line,
     *     including the line terminator.
     *
     * @param[in] lineFoldingStrategy
     *     This is the function to call that will determine
     *     where (if anywhere) to break up the input line.
     *
     * @return
     *     The output lines are returned.
     *
     * @retval {}
     *     This is returned if the line could not be folded into
     *     multiple lines.
     */
    std::vector< std::string > SplitLine(
        const std::string& line,
        const std::string& terminator,
        size_t limit,
        StringFoldingStrategy lineFoldingStrategy
    ) {
        std::vector< std::string > output;
        size_t currentLineStart = 0;
        size_t breakOffset = 0;
        while (currentLineStart < line.length()) {
            breakOffset = lineFoldingStrategy(line, currentLineStart);
            if (breakOffset == currentLineStart) {
                return {};
            }
            std::string part;
            if (!output.empty()) {
                part = " ";
            }
            part += line.substr(currentLineStart, breakOffset - currentLineStart);
            if (
                (part.length() < terminator.length())
                || (part.substr(part.length() - terminator.length()) != terminator)
                ) {
                part += terminator;
            }
            output.push_back(part);
            currentLineStart = breakOffset + 1;
        }
        return output;
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
                auto nextLineTerminator = rawMessage.find(CRLF, nextLineStart);
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

    /**
     * This is the Long Header Fields (2.2.3)
     * specified in RFC 2822 (https://tools.ietf.org/html/rfc2822).
     *
     * Each header field is logically a single line of characters comprising
     * the field name, the colon, and the field body.  For convenience
     * however, and to deal with the 998/78 character limitations per line,
     * the field body portion of a header field can be split into a multiple
     * line representation; this is called "folding".  The general rule is
     * that wherever this standard allows for folding white space (not
     * simply WSP characters), a CRLF may be inserted before any WSP.  For
     * example, the header field:
     *
     *      Subject: This is a test
     *
     *      can be represented as:
     *
     *          Subject: This
     *          is a test
     *
     * In the implementation, flag 'firstPart' is the one to help
     * us to realize the requirement.
     */
    std::string MessageHeaders::GenerateRawHeaders() const {
        std::ostringstream rawMessage;
        for (const auto& header : impl_->headers) {
            std::ostringstream lineBuffer;
            lineBuffer << header.name << ": " << header.value << CRLF;
            if (impl_->lineLengthLimit > 0) {
                bool firstPart = true;
                for (
                    const auto& part : SplitLine(
                        lineBuffer.str(), CRLF, impl_->lineLengthLimit,
                        [this, &firstPart](
                            const std::string& s,
                            size_t offset
                            ) {
                                if (s.length() - offset <= impl_->lineLengthLimit) {
                                    return s.length();
                                }
                                size_t lastBreakCandidate = offset;
                                const auto reservedCharacters = 2 + (firstPart ? 0 : 1);
                                for (size_t i = offset; i <= offset + impl_->lineLengthLimit - reservedCharacters; ++i) {
                                    if ((s[i] == ' ') || (s[i] == '\t')) {
                                        if (firstPart) {
                                            firstPart = false;
                                        }
                                        else {
                                            lastBreakCandidate = i;
                                        }
                                    }
                                }
                                return lastBreakCandidate;
                            }
                    )
                    ) {
                    rawMessage << part;
                }
            }
            else {
                rawMessage << lineBuffer.str();
            }
        }

        rawMessage << CRLF;
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

    void MessageHeaders::SetHeader(const HeaderName& name, const HeaderValue& value) {
        for (auto& header : impl_->headers) {
            if (header.name == name) {
                header.value = value;
                return;
            }
        }
        impl_->headers.emplace_back(name, value);
    }
}
