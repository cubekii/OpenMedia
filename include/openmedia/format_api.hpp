#pragma once

#include <functional>
#include <openmedia/error.h>
#include <openmedia/format_defs.h>
#include <openmedia/io.hpp>
#include <openmedia/macro.h>
#include <openmedia/packet.hpp>
#include <openmedia/result.hpp>
#include <openmedia/track.hpp>

namespace openmedia {

class OPENMEDIA_ABI Demuxer {
public:
  virtual ~Demuxer() = default;

  virtual auto open(std::unique_ptr<InputStream> input) -> OMError = 0;
  virtual void close() = 0;

  virtual auto tracks() const -> const std::vector<Track>& = 0;
  virtual auto readPacket() -> Result<Packet, OMError> = 0;
  virtual auto seek(int64_t timestamp_ns, int32_t stream_index) -> OMError = 0;
};

class OPENMEDIA_ABI Muxer {
public:
  virtual ~Muxer() = default;
};

struct OPENMEDIA_ABI FormatDescriptor {
  OMContainerId container_id = OM_CONTAINER_NONE;
  std::string_view name;
  std::string_view long_name;
  std::function<std::unique_ptr<Demuxer>()> demuxer_factory = {};
  std::function<std::unique_ptr<Muxer>()> muxer_factory = {};

  auto isDemuxing() const noexcept -> bool {
    return demuxer_factory != nullptr;
  }

  auto isMuxing() const noexcept -> bool {
    return muxer_factory != nullptr;
  }
};

} // namespace openmedia
