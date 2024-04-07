#pragma once

#include "NpCommon.hpp"
#include "NpTaskMap.hpp"
#include <boost/asio/thread_pool.hpp>
#include <grpc/event_engine/event_engine.h>

namespace gnp {

class NpEventEngine : public ge::EventEngine {
public:
  NpEventEngine();

  absl::StatusOr<std::unique_ptr<Listener>>
  CreateListener(Listener::AcceptCallback on_accept,
                 absl::AnyInvocable<void(absl::Status)> on_shutdown,
                 const ge::EndpointConfig &config,
                 std::unique_ptr<ge::MemoryAllocatorFactory>
                     memory_allocator_factory) override;
  ConnectionHandle Connect(OnConnectCallback on_connect,
                           const ResolvedAddress &addr,
                           const ge::EndpointConfig &args,
                           ge::MemoryAllocator memory_allocator,
                           Duration timeout) override;

  bool CancelConnect(ConnectionHandle handle) override;

  bool IsWorkerThread() override;

  absl::StatusOr<std::unique_ptr<DNSResolver>>
  GetDNSResolver(const DNSResolver::ResolverOptions &options) override;

  void Run(Closure *closure) override;

  void Run(absl::AnyInvocable<void()> closure) override;

  TaskHandle RunAfter(Duration when, Closure *closure) override;

  TaskHandle RunAfter(Duration when,
                      absl::AnyInvocable<void()> closure) override;
  bool Cancel(TaskHandle handle) override;

private:
  net::thread_pool tp_;
  std::shared_ptr<NpTaskMap> tasks_;
};

} // namespace gnp