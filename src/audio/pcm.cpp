#include <codecs.hpp>
#include <openmedia/audio.hpp>
#include <algorithm>
#include <cstring>
#include <vector>

namespace openmedia {

class PCMDecoder final : public Decoder {
  AudioFormat format_;

public:
  PCMDecoder() = default;
  ~PCMDecoder() override = default;

  auto configure(const DecoderOptions& options) -> OMError override {
    if (options.format.audio.channels == 0 || options.format.audio.sample_rate == 0) {
      return OM_CODEC_INVALID_PARAMS;
    }

    format_.sample_rate = options.format.audio.sample_rate;
    format_.channels = options.format.audio.channels;

    switch (options.format.codec_id) {
      case OM_CODEC_PCM_U8:    format_.sample_format = OM_SAMPLE_U8; break;
      case OM_CODEC_PCM_S16LE: format_.sample_format = OM_SAMPLE_S16; break;
      case OM_CODEC_PCM_S32LE: format_.sample_format = OM_SAMPLE_S32; break;
      case OM_CODEC_PCM_F32LE: format_.sample_format = OM_SAMPLE_F32; break;
      case OM_CODEC_PCM_F64LE: format_.sample_format = OM_SAMPLE_F64; break;
      default: return OM_CODEC_NOT_SUPPORTED;
    }

    return OM_SUCCESS;
  }

  auto getInfo() -> std::optional<DecodingInfo> override {
    DecodingInfo info = {};
    info.media_type = OM_MEDIA_AUDIO;
    info.audio_format = format_;
    return info;
  }

  auto decode(const Packet& packet) -> Result<std::vector<Frame>, OMError> override {
    if (packet.bytes.empty()) {
      return Err(OM_CODEC_DECODE_FAILED);
    }

    size_t bps = getBytesPerSample(format_.sample_format);
    size_t alignment = static_cast<size_t>(format_.channels) * bps;
    if (alignment == 0) {
      return Err(OM_CODEC_INVALID_PARAMS);
    }

    uint32_t nb_samples = static_cast<uint32_t>(packet.bytes.size() / alignment);
    if (nb_samples == 0) {
      return Ok(std::vector<Frame>{});
    }

    AudioSamples samples(format_, nb_samples);
    if (samples.buffer && samples.planes.count > 0 && samples.planes.data[0]) {
      std::memcpy(samples.planes.data[0], packet.bytes.data(), static_cast<size_t>(nb_samples) * alignment);
    }

    Frame frame;
    frame.pts = packet.pts;
    frame.dts = packet.dts;
    frame.data = std::move(samples);

    std::vector<Frame> frames;
    frames.push_back(std::move(frame));
    return Ok(std::move(frames));
  }

  void flush() override {}
};

static auto create_pcm_decoder() -> std::unique_ptr<Decoder> {
  return std::make_unique<PCMDecoder>();
}

const CodecDescriptor CODEC_PCM_S16LE = {
  .codec_id = OM_CODEC_PCM_S16LE,
  .type = OM_MEDIA_AUDIO,
  .name = "pcm_s16le",
  .long_name = "PCM Signed 16-bit",
  .vendor = "OpenMedia",
  .flags = NONE,
  .decoder_factory = create_pcm_decoder,
};

const CodecDescriptor CODEC_PCM_F32LE = {
  .codec_id = OM_CODEC_PCM_F32LE,
  .type = OM_MEDIA_AUDIO,
  .name = "pcm_f32le",
  .long_name = "PCM Float 32-bit",
  .vendor = "OpenMedia",
  .flags = NONE,
  .decoder_factory = create_pcm_decoder,
};

} // namespace openmedia
