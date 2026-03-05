#pragma once

#include <algorithm>
#include <functional>
#include <memory>
#include <openmedia/format_api.hpp>
#include <unordered_map>
#include <vector>

namespace openmedia {

struct OPENMEDIA_ABI FormatRegistry {
  std::unordered_map<OMContainerId, const FormatDescriptor*> format_table;

  FormatRegistry();
  ~FormatRegistry();

  auto registerFormat(const FormatDescriptor* descriptor, bool replace = false) noexcept -> bool;

  auto getFormat(OMContainerId format_id) const noexcept -> const FormatDescriptor*;

  auto getAllFormats() const -> std::vector<const FormatDescriptor*>;

  auto createDecoder(OMContainerId format_id) const noexcept -> std::unique_ptr<Demuxer>;

  auto createEncoder(OMContainerId format_id) const noexcept -> std::unique_ptr<Muxer>;

  auto hasFormat(OMContainerId format_id) const noexcept -> bool;

  auto hasDemuxer(OMContainerId format_id) const noexcept -> bool;

  auto hasMuxer(OMContainerId format_id) const noexcept -> bool;
};

OPENMEDIA_ABI
void registerBuiltInFormats(FormatRegistry* registry) noexcept;

} // namespace openmedia
