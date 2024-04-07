#include "NpEndpoint.hpp"
#include "NpCommon.hpp"
#include "boost/asio/write.hpp"

namespace gnp {

NpEndpoint::NpEndpoint(
    std::string &local_name, std::string &peer_name,
    winnet::named_pipe_protocol<net::thread_pool::executor_type>::pipe &&pipe,
    ge::MemoryAllocator &&alloc)
    : pipe_(std::move(pipe)), allocator_(std::move(alloc)), local_addr_(),
      peer_addr_() {
  {
    // prepare local addr
    sockaddr laddr = {};
    laddr.sa_family = AF_UNIX; // hack
    std::memcpy(laddr.sa_data, local_name.data(),
                std::min(sizeof(laddr.sa_data), local_name.size()));
    local_addr_ = ge::EventEngine::ResolvedAddress(&laddr, sizeof(laddr));
  }
  {
    // prepare peer addr
    sockaddr paddr = {};
    paddr.sa_family = AF_UNIX; // hack
    std::memcpy(paddr.sa_data, peer_name.data(),
                std::min(sizeof(paddr.sa_data), peer_name.size()));
    peer_addr_ = ge::EventEngine::ResolvedAddress(&paddr, sizeof(paddr));
  }
}

bool NpEndpoint::Read(absl::AnyInvocable<void(absl::Status)> on_read,
                      ge::SliceBuffer *buffer,
                      const ge::EventEngine::Endpoint::ReadArgs *args) {
  UNREFERENCED_PARAMETER(args); // TODO: read hint
  buffer->Clear();

  auto self = this->shared_from_this();
  // TODO: asio should read fixed bytes and copy in to buffer.
  pipe_.async_read_some(
      net::buffer(buffer_, sizeof(buffer_)),
      [self = std::move(self), on_read = std::move(on_read),
       buffer](boost::system::error_code ec, std::size_t len) mutable {
        if (ec.failed()) {
          auto status = absl::AbortedError("async_read_some failed");
          std::move(on_read)(std::move(status));
          return;
        }
        // BOOST_ASSERT_MSG(len != 0, "");
        auto slice = ge::Slice(self->allocator_.MakeSlice(len));

        std::memcpy((void *)slice.begin(), self->buffer_, len);

        buffer->AppendIndexed(std::move(slice));
        std::move(on_read)(absl::OkStatus());
      });
  // not sync
  return false;
}

bool NpEndpoint::Write(absl::AnyInvocable<void(absl::Status)> on_writable,
                       ge::SliceBuffer *data, const WriteArgs *args) {
  UNREFERENCED_PARAMETER(args); // TODO
  // convert buffer to vec

  write_content_.clear();
  for (size_t i = 0; i < data->Count(); i++) {
    auto slice = data->RefSlice(i);
    write_content_.insert(write_content_.end(), slice.cbegin(), slice.cend());
  }

  auto self = this->shared_from_this();
  // write vec to pipe
  net::async_write(
      pipe_, net::dynamic_buffer(write_content_),
      [self = std::move(self), on_writable = std::move(on_writable)](
          boost::system::error_code ec, std::size_t len) mutable {
        if (ec.failed()) {
          auto status = absl::AbortedError("async_write failed");
          std::move(on_writable)(std::move(status));
          return;
        }
        BOOST_ASSERT(len == self->write_content_.size());
        std::move(on_writable)(absl::OkStatus());
      });
  // not sync
  return false;
}

const ge::EventEngine::ResolvedAddress &NpEndpoint::GetPeerAddress() const {
  return peer_addr_;
}
const ge::EventEngine::ResolvedAddress &NpEndpoint::GetLocalAddress() const {
  return local_addr_;
}

} // namespace gnp