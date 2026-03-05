#pragma once

#include <functional>
#include <openmedia/codec_defs.h>
#include <openmedia/format_defs.h>
#include <span>
#include <vector>

namespace openmedia {

struct OPENMEDIA_ABI DetectedFormat {
  OMContainerId container;
  OMCodecId codec;

  auto isContainer() const -> bool { return container != OM_CONTAINER_NONE; }
  auto isRawCodec() const -> bool { return container == OM_CONTAINER_NONE && codec != OM_CODEC_NONE; }
  auto isUnknown() const -> bool { return container == OM_CONTAINER_NONE && codec == OM_CODEC_NONE; }

  static auto fromContainer(OMCodecId id) -> DetectedFormat {
    return DetectedFormat {id, OM_CODEC_NONE};
  }

  static auto fromCodec(OMCodecId id) -> DetectedFormat {
    return DetectedFormat {OM_CONTAINER_NONE, id};
  }

  static auto unknown() -> DetectedFormat {
    return DetectedFormat {OM_CONTAINER_NONE, OM_CODEC_NONE};
  }
};

using FormatDetectFn = std::function<DetectedFormat(std::span<const uint8_t>)>;

class OPENMEDIA_ABI FormatDetector {
  std::vector<FormatDetectFn> detectors_;

public:
  FormatDetector();

  auto detect(std::span<const uint8_t> data) const -> DetectedFormat;

  void addDetector(FormatDetectFn detector);
  void addStandardContainers();
  void addStandardAudio();
  void addStandardImages();
  void addStandardVideoBitstreams();
  void addAllStandard();
};

} // namespace openmedia
