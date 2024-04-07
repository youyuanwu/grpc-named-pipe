#pragma once

#include <boost/winasio/named_pipe/named_pipe_protocol.hpp>
#include <grpc/event_engine/event_engine.h>

namespace gnp {

namespace ge = grpc_event_engine::experimental;
namespace winnet = boost::winasio;
namespace net = boost::asio;

} // namespace gnp