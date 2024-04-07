#include "NpTaskMap.hpp"

namespace gnp {

// unique handle
intptr_t newTaskHandleNum() {
  static std::atomic<intptr_t> counter;
  intptr_t prev = counter.fetch_add(1);
  return prev;
}

ge::EventEngine::TaskHandle
NpTaskMap::Enqueue(std::shared_ptr<net::high_resolution_timer> timer) {
  std::unique_lock<std::mutex> lk(mtx_);
  TaskEntry e = TaskEntry{std::move(timer)};
  auto num = newTaskHandleNum();
  tasks_[num] = std::move(e);
  return ge::EventEngine::TaskHandle{0, num};
}

bool NpTaskMap::Cancel(ge::EventEngine::TaskHandle task) {
  std::unique_lock<std::mutex> lk(mtx_);
  auto it = tasks_.find(task.keys[1]);
  if (it == tasks_.end()) {
    // not found
    return false;
  }
  // TODO: this might need to be synchronized with timer backend.
  it->second.timer->cancel();
  tasks_.erase(it);
  return true;
}

void NpTaskMap::Finish(ge::EventEngine::TaskHandle task) {
  std::unique_lock<std::mutex> lk(mtx_);
  auto it = tasks_.find(task.keys[1]);
  if (it == tasks_.end()) {
    // not found. already cancelled
    return;
  }
  // TODO: might not need
  it->second.timer->wait();
  tasks_.erase(it);
}

} // namespace gnp