#pragma once
#include <type_traits>
#include <functional>

/**
 * @file
 * placeholder for std::bind
 */

namespace nng {
namespace placeholders {

struct Message_t {};
struct ErrorCode_t {};
struct Pipe_t {};
struct PipeEvent_t {};

/**
 * @class placeholders::Message
 * Placeholder that can be used in std::bind expressions to create `AsyncIO`
`CompletionHandler`s as a placeholder for the `nng::Message` argument of the
callback.
 * @code
 void foo(Message msg) {
 // [...]
 }

 // [...]
 nng::ReqSocket socket_;
 socket_.asyncSend(std::string("some message"), std::bind(foo,
nng::placholders::Message));
@endcode
*/
static Message_t Message;

/**
 * @class placeholders::Error
 * Placeholder that can be used in std::bind expressions to create `AsyncIO`
`CompletionHandler`s as a placeholder for the `std::error_code` argument of the
callback.
 * @code
 void foo(std::error_code ec) {
 // [...]
 }

 // [...]
 nng::ReqSocket socket_;
 socket_.asyncSend(std::string("some message"), std::bind(foo,
nng::placholders::ErrorCode));
@endcode
*/
static ErrorCode_t ErrorCode;

/**
 * @class placeholders::Pipe
 * Placeholder that can be used in std::bind expressions to create
 *  `EventHandler`s for `SocketBase::onPipeEvent()` as a placeholder for the
 * `Pipe` argument of the callback.
 */
static Pipe_t Pipe;

/**
 * @class placeholders::PipeEvent
 * Placeholder that can be used in std::bind expressions to create
 * `EventHandler`s for `SocketBase::onPipeEvent()` as a placeholder for the
 * `nng::PipeEvent` argument of the callback.
 */
static PipeEvent_t PipeEvent;

}  // namespace placeholders
}  // namespace nng

namespace std {
template <>
struct is_placeholder<nng::placeholders::ErrorCode_t>
    : public integral_constant<int, 1> {};
template <>
struct is_placeholder<nng::placeholders::Message_t>
    : public integral_constant<int, 2> {};
template <>
struct is_placeholder<nng::placeholders::PipeEvent_t>
    : public integral_constant<int, 1> {};
template <>
struct is_placeholder<nng::placeholders::Pipe_t>
    : public integral_constant<int, 2> {};
}  // namespace std
namespace std {}
