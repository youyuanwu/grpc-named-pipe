#pragma once

#include "NpCommon.hpp"

namespace gnp {

class NpListener : public ge::EventEngine::Listener {

  absl::StatusOr<int>
  Bind(const ge::EventEngine::ResolvedAddress &addr) override;
  absl::Status Start() override;
};

} // namespace gnp