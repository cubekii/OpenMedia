#include <cstring>
#include <util/demuxer_base.hpp>
#include <util/io_util.hpp>
#include <map>
#include <openmedia/audio.hpp>
#include <openmedia/format_api.hpp>
#include <openmedia/io.hpp>
#include <openmedia/packet.hpp>
#include <openmedia/result.hpp>
#include <openmedia/track.hpp>
#include <openmedia/video.hpp>
#include <vector>

namespace openmedia {

class MatroskaDemuxer final : public BaseDemuxer {

public:
  MatroskaDemuxer()  = default;
  ~MatroskaDemuxer() override {}

  auto open(std::unique_ptr<InputStream> input) -> OMError override {
    input_ = std::move(input);
    if (!input_ || !input_->isValid()) return OM_IO_INVALID_STREAM;

    return OM_SUCCESS;
  }

  void close() override {

  }

  auto readPacket() -> Result<Packet, OMError> override {
    return Err(OM_FORMAT_END_OF_FILE);
  }

  auto seek(int64_t timestamp_ns, int32_t stream_index) -> OMError override {
    return OM_FORMAT_PARSE_FAILED;
  }
};

auto create_matroska_demuxer() -> std::unique_ptr<Demuxer> {
  return std::make_unique<MatroskaDemuxer>();
}

const FormatDescriptor FORMAT_MATROSKA = {
    .container_id = OM_CONTAINER_MKV,
    .name = "matroska",
    .long_name = "Matroska / WebM",
    .demuxer_factory = [] { return create_matroska_demuxer(); },
    .muxer_factory = {},
};

} // namespace openmedia
