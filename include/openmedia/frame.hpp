#pragma once

#include <openmedia/audio.hpp>
#include <openmedia/video.hpp>
#include <variant>

namespace openmedia {

struct OPENMEDIA_ABI Frame {
  uint64_t pts = 0;
  uint64_t dts = 0;
  std::variant<AudioSamples, Picture> data;
};

}
