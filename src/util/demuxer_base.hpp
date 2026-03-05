#pragma once

#include <memory>
#include <openmedia/format_api.hpp>
#include <formats.hpp>
#include <vector>

namespace openmedia {

class BaseDemuxer : public Demuxer {
protected:
  std::unique_ptr<InputStream> input_;
  std::vector<Track> tracks_;

public:
  BaseDemuxer() = default;
  ~BaseDemuxer() override = default;

  void close() override {
    input_.reset();
    tracks_.clear();
  }

  auto tracks() const -> const std::vector<Track>& override {
    return tracks_;
  }
};

} // namespace openmedia
