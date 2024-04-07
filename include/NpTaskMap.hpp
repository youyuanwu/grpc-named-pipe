#pragma once

#include "NpCommon.hpp"
#include <boost/asio/high_resolution_timer.hpp>
#include <memory>

namespace gnp {

class NpTaskMap {
public:
  ge::EventEngine::TaskHandle
  Enqueue(std::shared_ptr<net::high_resolution_timer> timer);
  // return true if success cancel, false if already run.
  bool Cancel(ge::EventEngine::TaskHandle task);
  void Finish(ge::EventEngine::TaskHandle task);

private:
  struct TaskEntry {
    std::shared_ptr<net::high_resolution_timer> timer;
  };

  std::map<intptr_t, TaskEntry> tasks_;
  std::mutex mtx_;
};

} // namespace gnp