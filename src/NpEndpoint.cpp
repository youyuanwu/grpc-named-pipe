#include "NpEndpoint.hpp"
#include "NpCommon.hpp"

namespace gnp {

void NpEndpoint::Read(absl::AnyInvocable<void(absl::Status)> on_read,
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
          auto status = absl::UnavailableError("async_read_some failed");
          std::move(on_read)(std::move(status));
          return;
        }
        // BOOST_ASSERT_MSG(len != 0, "");
        auto slice = ge::Slice(self->allocator_.MakeSlice(len));

        std::memcpy((void *)slice.begin(), self->buffer_, len);

        buffer->AppendIndexed(std::move(slice));
        std::move(on_read)(absl::OkStatus());
      });
}

} // namespace gnp