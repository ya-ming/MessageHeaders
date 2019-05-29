#ifndef MESSAGE_HEADER_HPP
#define MESSAGE_HEADER_HPP

/**
 * @file MessageHeaders.hpp
 *
 * This module declares the MessageHeaders::MessageHeaders class
 * 
 * 2019 by YaMing Wu
 * 
 */

#include <memory>
#include <string>
#include <vector>

namespace MessageHeaders
{
    /**
     * This class represents an MessageHeaders
     * as defined in RFC 2822 (https://tools.ietf.org/html/rfc2822) 
     */
    class MessageHeaders {

    public:
        /**
         * This is how we handle the name of a message header.
         */
        typedef std::string HeaderName;

        /**
         * This is how we handle the value of a message header.
         */
        typedef std::string HeaderValue;

        /**
         * This represents a single header of the internet message.
         */
        struct Header {
            /**
             * This is the part of a header that comes before the colon.
             * It identifies the purpose of the header.
             */
            HeaderName name;

            /**
            * This is the part of a header that comes after the colon.
            * It provides the value, setting, or context whose meaning
            * depends on the header name.
            */
            HeaderValue value;

            // Methods
            /**
             * This constructor initializes the header's components.
             *
             * @param[in] newName
             *     This is the part of a header that comes before the colon.
             *     It identifies the purpose of the header.
             *
             * @param[in] newValue
             *     This is the part of a header that comes after the colon.
             *     It provides the value, setting, or context whose meaning
             *     depends on the header name.
             */
            Header(
                const HeaderName& newName,
                const HeaderValue& newValue
            );
        };

        /**
         * This represents the collection of all headers of the message.
         */
        typedef std::vector<Header> Headers;

        // Lifecycle management
    public:
        ~MessageHeaders();
        MessageHeaders(const MessageHeaders&) = delete;
        MessageHeaders(MessageHeaders&&) = delete;
        MessageHeaders& operator=(const MessageHeaders&) = delete;
        MessageHeaders& operator=(MessageHeaders&&) = delete;

        // public methods
    public:
        /**
         * This is the default constructor.
         */
        MessageHeaders();

        /**
         * This method determines the headers and body
         * of the message by parsing the raw message from a string.
         *
         *@param[in] rawMessage
         *     This is the string rendering of the message to parse.
         *
         * @param[out] bodyOffset
         *     This is where to store the offset into the given
         *     raw message where the headers ended and the body,
         *if any, begins.
         *
         * @return
         *     An indication of whether or not the message was
         *     parsed successfully is returned.
         */
        bool ParseRawMessage(const std::string& rawMessage, size_t& bodyOffset);

        /**
         * This method determines the headers
         * of the message by parsing the raw message from a string.
         *
         * @param[in] rawMessage
         *     This is the string rendering of the message to parse.
         *
         * @return
         *     An indication of whether or not the message was
         *     parsed successfully is returned.
         */
        bool ParseRawMessage(const std::string& rawMessage);


        Headers GetAll() const;

        bool HasHeader(const HeaderName& name) const;

        /**
         * This method returns the value for the header with the
         * given name in the message.
         *
         * @param[in] name
         *      This is the name of the header whose value should be returned.
         *
         * @return
         *      The value of the given header is returned.
         */
        HeaderValue GetHeaderValue(const HeaderName& name) const;

        /**
         * This method constructs and returns the raw string
         * headers based on the headers that
         * have been collected in the object.
         *
         * @return
         *      Ths raw string internet message based on the headers 
         *      that have been collected in the object is returned.
         */
        std::string GenerateRawHeaders() const;

        // Private properties
    private:
        /**
         * This is the type of structure that contains the private
         * properties of the instance.  It is defined in the implementation
         * and declared here to ensure that it is scoped inside the class.
         */
        struct Impl;

        /**
         * This contains the private properties of the instance.
         */
        std::unique_ptr< struct Impl > impl_;
    };
} // namespace MessageHeaders


#endif
