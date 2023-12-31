#pragma once

#include "NpCommon.hpp"
#include <grpc/event_engine/event_engine.h>

#include <boost/winasio/named_pipe/named_pipe_protocol.hpp>
#include <memory>

namespace gnp {

class NpEndpoint : public ge::EventEngine::Endpoint,
                   public std::enable_shared_from_this<NpEndpoint> {
public:
  // TODO: constructor
  void Read(absl::AnyInvocable<void(absl::Status)> on_read,
            ge::SliceBuffer *buffer, const ReadArgs *args) override;

private:
  winnet::named_pipe_protocol<net::io_context::executor_type>::pipe pipe_;
  ge::MemoryAllocator allocator_;
  char buffer_[1024];
};

} // namespace gnp