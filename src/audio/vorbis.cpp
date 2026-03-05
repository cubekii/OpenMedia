#include <codecs.hpp>
#include <openmedia/audio.hpp>
#include <vorbis/codec.h>
#include <ogg/ogg.h>
#include <algorithm>
#include <cstring>
#include <vector>

namespace openmedia {

class VorbisDecoder final : public Decoder {
  vorbis_info vi_ = {};
  vorbis_comment vc_ = {};
  vorbis_dsp_state vd_ = {};
  vorbis_block vb_ = {};
  int header_packets_ = 0;
  long packet_count_ = 0;
  bool initialized_ = false;
  AudioFormat output_format_;

public:
  VorbisDecoder() {
    vorbis_info_init(&vi_);
    vorbis_comment_init(&vc_);
  }

  ~VorbisDecoder() override {
    if (initialized_) {
      vorbis_block_clear(&vb_);
      vorbis_dsp_clear(&vd_);
    }
    vorbis_comment_clear(&vc_);
    vorbis_info_clear(&vi_);
  }

  auto configure(const DecoderOptions& options) -> OMError override {
    if (options.format.codec_id != OM_CODEC_VORBIS) {
      return OM_CODEC_INVALID_PARAMS;
    }
    return OM_SUCCESS;
  }

  auto getInfo() -> std::optional<DecodingInfo> override {
    if (!initialized_) return std::nullopt;

    output_format_.sample_rate = static_cast<uint32_t>(vi_.rate);
    output_format_.channels = static_cast<uint32_t>(vi_.channels);
    output_format_.sample_format = OM_SAMPLE_F32;

    DecodingInfo info;
    info.media_type = OM_MEDIA_AUDIO;
    info.audio_format = output_format_;
    return info;
  }

  auto decode(const Packet& packet) -> Result<std::vector<Frame>, OMError> override {
    ogg_packet op;
    op.packet = const_cast<unsigned char*>(packet.bytes.data());
    op.bytes = static_cast<long>(packet.bytes.size());
    op.b_o_s = (header_packets_ == 0);
    op.e_o_s = 0;
    op.granulepos = packet.pts;
    op.packetno = packet_count_++;

    if (header_packets_ < 3) {
      int ret = vorbis_synthesis_headerin(&vi_, &vc_, &op);
      if (ret == 0) {
        header_packets_++;
        if (header_packets_ == 3) {
          vorbis_synthesis_init(&vd_, &vi_);
          vorbis_block_init(&vd_, &vb_);
          initialized_ = true;
        }
      }
      return Ok(std::vector<Frame>{});
    }

    if (!initialized_) {
      return Ok(std::vector<Frame>{});
    }

    std::vector<Frame> frames;
    if (vorbis_synthesis(&vb_, &op) == 0) {
      vorbis_synthesis_blockin(&vd_, &vb_);

      float** pcm;
      int samples = vorbis_synthesis_pcmout(&vd_, &pcm);
      if (samples > 0) {
        output_format_.sample_rate = static_cast<uint32_t>(vi_.rate);
        output_format_.channels = static_cast<uint32_t>(vi_.channels);
        output_format_.sample_format = OM_SAMPLE_F32;

        AudioSamples samples_fmt(output_format_, static_cast<uint32_t>(samples));
        float* dst = reinterpret_cast<float*>(samples_fmt.planes.data[0]);
        for (int i = 0; i < samples; ++i) {
          for (int c = 0; c < vi_.channels; ++c) {
            *dst++ = pcm[c][i];
          }
        }

        Frame frame;
        frame.pts = packet.pts;
        frame.dts = packet.dts;
        frame.data = std::move(samples_fmt);
        frames.push_back(std::move(frame));

        vorbis_synthesis_read(&vd_, samples);
      }
    }

    return Ok(std::move(frames));
  }

  void flush() override {}
};

auto create_vorbis_decoder() -> std::unique_ptr<Decoder> {
  return std::make_unique<VorbisDecoder>();
}

const CodecDescriptor CODEC_VORBIS = {
  .codec_id = OM_CODEC_VORBIS,
  .type = OM_MEDIA_AUDIO,
  .name = "vorbis",
  .long_name = "Vorbis audio decoder",
  .vendor = "Xiph.Org",
  .flags = NONE,
  .decoder_factory = create_vorbis_decoder,
};

} // namespace openmedia
