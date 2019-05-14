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

namespace MessageHeaders
{
    /**
     * This class represents an MessageHeaders
     * as defined in RFC 2822 (https://tools.ietf.org/html/rfc2822) 
     */
    class MessageHeaders {
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