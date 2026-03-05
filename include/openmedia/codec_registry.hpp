#pragma once

#include <algorithm>
#include <functional>
#include <memory>
#include <openmedia/codec_api.hpp>
#include <unordered_map>
#include <vector>

namespace openmedia {

struct OPENMEDIA_ABI CodecRegistry {
  std::unordered_multimap<OMCodecId, const CodecDescriptor*> codec_table;
  std::unordered_map<std::string_view, const CodecDescriptor*> name_table;

  CodecRegistry();
  ~CodecRegistry();

  auto registerCodec(const CodecDescriptor* descriptor) noexcept -> bool;

  auto getCodec(OMCodecId codec_id) const noexcept -> const CodecDescriptor*;

  auto getCodecByName(std::string_view name) const noexcept -> const CodecDescriptor*;

  auto getAllCodecs() const -> std::vector<const CodecDescriptor*>;

  auto getCodecsByType(OMMediaType type) const -> std::vector<const CodecDescriptor*>;

  auto getCodecsByCodecId(OMCodecId codec_id) const -> std::vector<const CodecDescriptor*>;

  auto createDecoder(OMCodecId codec_id) const noexcept -> std::unique_ptr<Decoder>;

  auto createEncoder(OMCodecId codec_id) const noexcept -> std::unique_ptr<Encoder>;

  auto hasCodec(OMCodecId codec_id) const noexcept -> bool;

  auto hasDecoder(OMCodecId codec_id) const noexcept -> bool;

  auto hasEncoder(OMCodecId codec_id) const noexcept -> bool;
};

OPENMEDIA_ABI
void registerBuiltInCodecs(CodecRegistry* registry) noexcept;

} // namespace openmedia
