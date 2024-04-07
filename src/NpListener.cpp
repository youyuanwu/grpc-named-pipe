#include "NpListener.hpp"
#include "NpCommon.hpp"

namespace gnp {

absl::StatusOr<int>
NpListener::Bind(const ge::EventEngine::ResolvedAddress &addr) {
  UNREFERENCED_PARAMETER(addr);
  BOOST_ASSERT(false);
  return 0;
}

absl::Status NpListener::Start() {
  BOOST_ASSERT(false);
  return absl::OkStatus();
}

} // namespace gnp