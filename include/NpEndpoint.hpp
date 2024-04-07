#pragma once

#include "NpCommon.hpp"
#include <grpc/event_engine/event_engine.h>

#include "boost/asio/thread_pool.hpp"
#include <boost/winasio/named_pipe/named_pipe_protocol.hpp>
#include <memory>

namespace gnp {

class NpEndpoint : public ge::EventEngine::Endpoint,
                   public std::enable_shared_from_this<NpEndpoint> {
public:
  NpEndpoint(
      std::string &local_name, std::string &peer_name,
      winnet::named_pipe_protocol<net::thread_pool::executor_type>::pipe &&pipe,
      ge::MemoryAllocator &&alloc);

  NpEndpoint(NpEndpoint &&other) = default;

  bool Read(absl::AnyInvocable<void(absl::Status)> on_read,
            ge::SliceBuffer *buffer, const ReadArgs *args) override;

  bool Write(absl::AnyInvocable<void(absl::Status)> on_writable,
             ge::SliceBuffer *data, const WriteArgs *args) override;

  const ge::EventEngine::ResolvedAddress &GetPeerAddress() const override;
  const ge::EventEngine::ResolvedAddress &GetLocalAddress() const override;

private:
  winnet::named_pipe_protocol<net::thread_pool::executor_type>::pipe pipe_;
  ge::MemoryAllocator allocator_;
  // read buff
  char buffer_[1024];
  std::vector<std::uint8_t> write_content_;

  ge::EventEngine::ResolvedAddress local_addr_;
  ge::EventEngine::ResolvedAddress peer_addr_;
};

} // namespace gnp