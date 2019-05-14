/**
 * @file MessageHeaders.cpp
 * 
 * This module contains the implementation of the MessageHeaders::MessageHeaders class.
 * 
 * 2019 by YaMing Wu
 */

#include <MessageHeaders/MessageHeaders.hpp>

namespace MessageHeaders {
    /**
     * This contains the private properties of a MessageHeaders instance.
     */
    struct MessageHeaders::Impl {

    };

    MessageHeaders::~MessageHeaders() = default;

    MessageHeaders::MessageHeaders() 
        : impl_(new Impl) {

    }
}
