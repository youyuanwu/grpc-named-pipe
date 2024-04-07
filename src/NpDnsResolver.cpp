#include "NpDnsResolver.hpp"
#include <absl/strings/str_cat.h>
#include <afunix.h> // for longer addr buf
#include <grpc/event_engine/event_engine.h>
#include <thread>

namespace gnp {

void NpDnsResolver::LookupHostname(LookupHostnameCallback on_resolve,
                                   absl::string_view name,
                                   absl::string_view default_port) {
  std::string name_copy(name);
  // TODO: use the thread pool
  // synchronous will have a mutex deadlock warning in the parent
  std::thread([on_resolve = std::move(on_resolve),
               name_copy = std::move(name_copy)]() mutable {
    // hack
    sockaddr_un addr_raw;
    if (name_copy.size() + 1 > sizeof(addr_raw.sun_path)) {
      std::move(on_resolve)(absl::InvalidArgumentError(
          absl::StrCat("addr too long: ", name_copy)));
      return;
    }
    addr_raw.sun_family = AF_UNIX;
    name_copy.copy(addr_raw.sun_path, name_copy.size());
    addr_raw.sun_path[name_copy.size()] = 0; // terminator
    ge::EventEngine::ResolvedAddress addr(
        reinterpret_cast<const sockaddr *>(&addr_raw), sizeof(addr_raw));
    std::cout << "debug host name: " << name_copy << std::endl;
    std::move(on_resolve)(
        std::vector<ge::EventEngine::ResolvedAddress>{std::move(addr)});
  }).detach();
}
/// Asynchronously perform an SRV record lookup.
///
/// \a on_resolve has the same meaning and expectations as \a
/// LookupHostname's \a on_resolve callback.
void NpDnsResolver::LookupSRV(LookupSRVCallback on_resolve,
                              absl::string_view name) {
  assert(false);
}
/// Asynchronously perform a TXT record lookup.
///
/// \a on_resolve has the same meaning and expectations as \a
/// LookupHostname's \a on_resolve callback.
void NpDnsResolver::LookupTXT(LookupTXTCallback on_resolve,
                              absl::string_view name) {
  assert(false);
}

} // namespace gnp