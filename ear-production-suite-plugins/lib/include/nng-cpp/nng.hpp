#pragma once

#include "protocols/pull.hpp"
#include "protocols/push.hpp"
#include "protocols/req.hpp"
#include "protocols/rep.hpp"
#include "protocols/pub.hpp"
#include "protocols/sub.hpp"

/**
 * @brief A C++ wrapper for `nng`
 * https://nanomsg.github.io/nng/index.html
 *
 * Currently, this not a complete wrapper, it merily
 * implements the features currently needed by us.
 * Right now, there's also no intention to upgrade
 * this to a full-fledge nng wrapper.
 *
 * There exists another C++ wrapper project `nngpp`,
 * but this project "only" provides simple class wrappers
 * around nng C-style API.
 *
 * For our project, a higher level of abstraction is
 * envisioned, especially the possibility to work with
 * modern C++ constructs like std::function<> for callbacks
 * and so on.
 */
namespace nng {}
