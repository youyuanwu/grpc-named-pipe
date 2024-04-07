#include "NpEventEngine.hpp"
#include "NpDnsResolver.hpp"
#include "NpEndpoint.hpp"
#include "NpListener.hpp"

#include <absl/strings/str_cat.h>
#include <boost/asio/high_resolution_timer.hpp>
#include <boost/asio/post.hpp>
#include <boost/winasio/named_pipe/named_pipe_protocol.hpp>

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
  UNREFERENCED_PARAMETER(args);

  winnet::named_pipe_protocol<decltype(tp_.get_executor())>::pipe p(
      tp_.get_executor());
  BOOST_ASSERT_MSG(addr.address()->sa_family == AF_UNIX, "AF not right");
  std::string endpoint(addr.address()->sa_data);
  boost::system::error_code ec;
  long long ms =
      std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count();
  p.connect(endpoint, ec, static_cast<std::uint32_t>(ms));
  if (ec.failed()) {
    // avoid deadlock
    this->Run([on_connect = std::move(on_connect), endpoint, ec]() mutable {
      auto status = absl::UnavailableError(
          absl::StrCat("Fail to connect to np: ", endpoint, " ", ec.message()));
      std::move(on_connect)(status);
    });
    return ConnectionHandle{};
  }

  this->Run([on_connect = std::move(on_connect), endpoint, p = std::move(p),
             ma = std::move(memory_allocator)]() mutable {
    auto ep = std::make_unique<NpEndpoint>(std::string{}, // local name
                                           endpoint,      // peer name
                                           std::move(p), std::move(ma));
    std::move(on_connect)(std::move(ep));
  });

  static std::atomic_uint32_t counter;
  counter++;
  return ge::EventEngine::ConnectionHandle{0, counter};
}

bool NpEventEngine::CancelConnect(ConnectionHandle handle) {
  if (handle == ConnectionHandle{}) {
    // empty handle
    return false;
  }
  // connect is sync so there is no cancel
  UNREFERENCED_PARAMETER(handle);
  BOOST_ASSERT(false);
  return false;
}

bool NpEventEngine::IsWorkerThread() { return true; }

absl::StatusOr<std::unique_ptr<ge::EventEngine::DNSResolver>>
NpEventEngine::GetDNSResolver(const DNSResolver::ResolverOptions &options) {
  std::cout << "Debug: DnsServer" << options.dns_server << std::endl;
  UNREFERENCED_PARAMETER(options);
  return std::make_unique<NpDnsResolver>();
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