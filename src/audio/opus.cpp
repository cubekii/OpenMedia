#include <codecs.hpp>
#include <openmedia/audio.hpp>
#include <opus.h>
#include <algorithm>
#include <cstring>
#include <vector>

namespace openmedia {

class OpusDecoder final : public Decoder {
  ::OpusDecoder* decoder_ = nullptr;
  int channels_ = 0;
  int sample_rate_ = 0;
  AudioFormat output_format_;

public:
  OpusDecoder() {
    output_format_.sample_format = OM_SAMPLE_F32;
    output_format_.bits_per_sample = 32;
  }

  ~OpusDecoder() override {
    if (decoder_) {
      opus_decoder_destroy(decoder_);
    }
  }

  auto configure(const DecoderOptions& options) -> OMError override {
    if (options.format.codec_id != OM_CODEC_OPUS) {
      return OM_CODEC_INVALID_PARAMS;
    }
    return OM_SUCCESS;
  }

  auto getInfo() -> std::optional<DecodingInfo> override {
    if (!decoder_) return std::nullopt;

    output_format_.sample_rate = static_cast<uint32_t>(sample_rate_);
    output_format_.channels = static_cast<uint32_t>(channels_);

    DecodingInfo info;
    info.media_type = OM_MEDIA_AUDIO;
    info.audio_format = output_format_;
    return info;
  }

  auto decode(const Packet& packet) -> Result<std::vector<Frame>, OMError> override {
    if (packet.bytes.size() >= 8 && std::memcmp(packet.bytes.data(), "OpusHead", 8) == 0) {
      if (packet.bytes.size() < 19) {
        return Ok(std::vector<Frame>{});
      }
      channels_ = packet.bytes.data()[9];
      sample_rate_ = 48000;

      int error = 0;
      if (decoder_) opus_decoder_destroy(decoder_);
      decoder_ = opus_decoder_create(sample_rate_, channels_, &error);
      if (error != OPUS_OK) {
        return Err(OM_CODEC_OPEN_FAILED);
      }
      return Ok(std::vector<Frame>{});
    }

    if (packet.bytes.size() >= 8 && std::memcmp(packet.bytes.data(), "OpusTags", 8) == 0) {
      return Ok(std::vector<Frame>{});
    }

    if (!decoder_) {
      return Ok(std::vector<Frame>{});
    }

    const int MAX_SAMPLES = 5760;
    output_format_.sample_rate = static_cast<uint32_t>(sample_rate_);
    output_format_.channels = static_cast<uint32_t>(channels_);

    AudioSamples samples_fmt(output_format_, MAX_SAMPLES);
    int samples = opus_decode_float(decoder_, packet.bytes.data(),
                                    static_cast<opus_int32>(packet.bytes.size()),
                                    reinterpret_cast<float*>(samples_fmt.planes.data[0]),
                                    MAX_SAMPLES, 0);

    if (samples > 0) {
      samples_fmt.nb_samples = static_cast<uint32_t>(samples);

      Frame frame;
      frame.pts = packet.pts;
      frame.dts = packet.dts;
      frame.data = std::move(samples_fmt);

      std::vector<Frame> frames;
      frames.push_back(std::move(frame));
      return Ok(std::move(frames));
    }

    return Ok(std::vector<Frame>{});
  }

  void flush() override {
    if (decoder_) {
      opus_decoder_ctl(decoder_, OPUS_RESET_STATE);
    }
  }
};

auto create_opus_decoder() -> std::unique_ptr<Decoder> {
  return std::make_unique<OpusDecoder>();
}

const CodecDescriptor CODEC_OPUS = {
  .codec_id = OM_CODEC_OPUS,
  .type = OM_MEDIA_AUDIO,
  .name = "opus",
  .long_name = "Opus audio decoder",
  .vendor = "Xiph.Org",
  .flags = NONE,
  .decoder_factory = create_opus_decoder,
};

} // namespace openmedia
