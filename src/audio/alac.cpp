#include <ALACBitUtilities.h>
#include <ALACDecoder.h>
#include <cstring>
#include <openmedia/audio.hpp>
#include <codecs.hpp>
#include <vector>

namespace openmedia {

class ALACDecoder final : public Decoder {
  ::ALACDecoder decoder_;
  AudioFormat output_format_;
  bool initialized_ = false;

public:
  ALACDecoder() = default;

  ~ALACDecoder() override = default;

  auto configure(const DecoderOptions& options) -> OMError override {
    if (options.format.codec_id != OM_CODEC_ALAC) {
      return OM_CODEC_INVALID_PARAMS;
    }

    if (options.extradata.empty()) {
      return OM_CODEC_INVALID_PARAMS;
    }

    if (decoder_.Init(options.extradata.data(), static_cast<uint32_t>(options.extradata.size())) != 0) {
      return OM_CODEC_OPEN_FAILED;
    }

    output_format_.channels = decoder_.mConfig.numChannels;
    output_format_.sample_rate = decoder_.mConfig.sampleRate;
    output_format_.planar = false;

    switch (decoder_.mConfig.bitDepth) {
      case 16:
        output_format_.sample_format = OM_SAMPLE_S16;
        break;
      case 24:
        output_format_.sample_format = OM_SAMPLE_S32;
        break;
      case 32:
        output_format_.sample_format = OM_SAMPLE_S32;
        break;
      default:
        return OM_CODEC_INVALID_PARAMS;
    }

    initialized_ = true;
    return OM_SUCCESS;
  }

  auto getInfo() -> std::optional<DecodingInfo> override {
    if (!initialized_) return std::nullopt;

    DecodingInfo info = {};
    info.media_type = OM_MEDIA_AUDIO;
    info.audio_format = output_format_;
    return info;
  }

  auto decode(const Packet& packet) -> Result<std::vector<Frame>, OMError> override {
    BitBuffer bits;
    BitBufferInit(&bits, packet.bytes.data(), static_cast<uint32_t>(packet.bytes.size()));

    uint32_t frame_length = decoder_.mConfig.frameLength;
    uint32_t channels = decoder_.mConfig.numChannels;

    AudioSamples samples(output_format_, frame_length);
    uint32_t out_num_samples = 0;

    if (decoder_.Decode(&bits, samples.planes.data[0], frame_length, channels, &out_num_samples) != 0) {
      return Err(OM_CODEC_DECODE_FAILED);
    }

    if (out_num_samples != frame_length) {
      samples.nb_samples = out_num_samples;
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

const CodecDescriptor CODEC_ALAC = {
  .codec_id = OM_CODEC_ALAC,
  .type = OM_MEDIA_AUDIO,
  .name = "alac",
  .long_name = "Apple Lossless Audio Codec",
  .vendor = "Apple",
  .flags = NONE,
  .decoder_factory = []{ return std::make_unique<ALACDecoder>(); },
};

} // namespace openmedia
