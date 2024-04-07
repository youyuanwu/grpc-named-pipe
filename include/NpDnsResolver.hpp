#pragma once
#include "NpCommon.hpp"

namespace gnp {

class NpEventEngine;

class NpDnsResolver : public ge::EventEngine::DNSResolver {
public:
  void LookupHostname(LookupHostnameCallback on_resolve, absl::string_view name,
                      absl::string_view default_port) override;
  /// Asynchronously perform an SRV record lookup.
  ///
  /// \a on_resolve has the same meaning and expectations as \a
  /// LookupHostname's \a on_resolve callback.
  void LookupSRV(LookupSRVCallback on_resolve, absl::string_view name) override;
  /// Asynchronously perform a TXT record lookup.
  ///
  /// \a on_resolve has the same meaning and expectations as \a
  /// LookupHostname's \a on_resolve callback.
  void LookupTXT(LookupTXTCallback on_resolve, absl::string_view name) override;
};

} // namespace gnp