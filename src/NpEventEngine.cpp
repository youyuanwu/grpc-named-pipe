#include "NpEventEngine.hpp"
#include "NpListener.hpp"

#include <boost/asio/high_resolution_timer.hpp>
#include <boost/asio/post.hpp>

namespace gnp {

NpEventEngine::NpEventEngine()
    : tp_(1), tasks_(std::make_shared<NpTaskMap>()) {}

absl::StatusOr<std::unique_ptr<ge::EventEngine::Listener>>
NpEventEngine::CreateListener(
    Listener::AcceptCallback on_accept,
    absl::AnyInvocable<void(absl::Status)> on_shutdown,
    const ge::EndpointConfig &config,
    std::unique_ptr<ge::MemoryAllocatorFactory> memory_allocator_factory) {

  UNREFERENCED_PARAMETER(config);
  UNREFERENCED_PARAMETER(on_accept);
  UNREFERENCED_PARAMETER(on_shutdown);
  UNREFERENCED_PARAMETER(memory_allocator_factory);

  auto l = std::make_unique<NpListener>();
  // TODO: impl server
  BOOST_ASSERT(false);
  return l;
}

ge::EventEngine::ConnectionHandle
NpEventEngine::Connect(OnConnectCallback on_connect,
                       const ResolvedAddress &addr,
                       const ge::EndpointConfig &args,
                       ge::MemoryAllocator memory_allocator, Duration timeout) {
  UNREFERENCED_PARAMETER(on_connect);
  UNREFERENCED_PARAMETER(addr);
  UNREFERENCED_PARAMETER(args);
  UNREFERENCED_PARAMETER(memory_allocator);
  UNREFERENCED_PARAMETER(timeout);
  // client not implemented
  BOOST_ASSERT(false);
  return ge::EventEngine::ConnectionHandle{};
}

bool NpEventEngine::CancelConnect(ConnectionHandle handle) {
  UNREFERENCED_PARAMETER(handle);
  BOOST_ASSERT(false);
  return false;
}

bool NpEventEngine::IsWorkerThread() { return true; }

std::unique_ptr<ge::EventEngine::DNSResolver>
NpEventEngine::GetDNSResolver(const DNSResolver::ResolverOptions &options) {
  UNREFERENCED_PARAMETER(options);
  BOOST_ASSERT(false);
  return nullptr;
}

void NpEventEngine::Run(Closure *closure) {
  net::post(tp_, [&]() { closure->Run(); });
}

void NpEventEngine::Run(absl::AnyInvocable<void()> closure) {
  net::post(tp_, [c = std::move(closure)]() mutable { std::move(c)(); });
}

ge::EventEngine::TaskHandle NpEventEngine::RunAfter(Duration when,
                                                    Closure *closure) {
  std::chrono::steady_clock::time_point timepoint =
      std::chrono::steady_clock::now() + when;
  auto timer =
      std::make_shared<net::high_resolution_timer>(tp_.executor(), timepoint);

  std::weak_ptr<NpTaskMap> tasks = tasks_;
  auto handle = tasks_->Enqueue(timer);
  timer->async_wait([=](boost::system::error_code ec) {
    if (ec == boost::asio::error::operation_aborted) {
      // cancelled
      return;
    }
    closure->Run();
    if (auto t = tasks.lock()) {
      t->Finish(handle);
    } else {
      BOOST_ASSERT_MSG(false, "tasks map deleted");
    }
  });
  return handle;
}

ge::EventEngine::TaskHandle
NpEventEngine::RunAfter(Duration when, absl::AnyInvocable<void()> closure) {
  std::chrono::steady_clock::time_point timepoint =
      std::chrono::steady_clock::now() + when;
  auto timer =
      std::make_shared<net::high_resolution_timer>(tp_.executor(), timepoint);

  std::weak_ptr<NpTaskMap> tasks = tasks_;
  auto handle = tasks_->Enqueue(timer);
  timer->async_wait(
      [=, c = std::move(closure)](boost::system::error_code ec) mutable {
        if (ec == boost::asio::error::operation_aborted) {
          // cancelled
          return;
        }
        std::move(c)();
        if (auto t = tasks.lock()) {
          t->Finish(handle);
        } else {
          BOOST_ASSERT_MSG(false, "tasks map deleted");
        }
      });
  return handle;
}

bool NpEventEngine::Cancel(TaskHandle handle) { return tasks_->Cancel(handle); }

} // namespace gnp