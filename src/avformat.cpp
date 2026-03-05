#include <libavformat/avformat.h>
#include <algorithm>
#include <cstring>
#include <util/demuxer_base.hpp>
#include <future>
#include <openmedia/format_api.hpp>
#include <openmedia/packet.hpp>
#include <openmedia/track.hpp>

namespace openmedia {

static auto avMediaTypeToMediaType(AVMediaType media_type) -> OMMediaType {
  switch (media_type) {
    case AVMEDIA_TYPE_VIDEO: return OM_MEDIA_VIDEO;
    case AVMEDIA_TYPE_AUDIO: return OM_MEDIA_AUDIO;
    case AVMEDIA_TYPE_SUBTITLE: return OM_MEDIA_SUBTITLE;
    case AVMEDIA_TYPE_DATA: return OM_MEDIA_DATA;
    default: return OM_MEDIA_NONE;
  }
}

static auto avCodecIdToCodecId(AVCodecID id) -> OMCodecId {
  switch (id) {
    // Video
    case AV_CODEC_ID_H264: return OM_CODEC_H264;
    case AV_CODEC_ID_HEVC: return OM_CODEC_H265;
    case AV_CODEC_ID_VP8: return OM_CODEC_VP8;
    case AV_CODEC_ID_VP9: return OM_CODEC_VP9;
    case AV_CODEC_ID_AV1: return OM_CODEC_AV1;
    case AV_CODEC_ID_MPEG4: return OM_CODEC_MPEG4;
    // Audio
    case AV_CODEC_ID_AAC: return OM_CODEC_AAC;
    case AV_CODEC_ID_MP3: return OM_CODEC_MP3;
    case AV_CODEC_ID_OPUS: return OM_CODEC_OPUS;
    case AV_CODEC_ID_VORBIS: return OM_CODEC_VORBIS;
    case AV_CODEC_ID_FLAC: return OM_CODEC_FLAC;
    case AV_CODEC_ID_PCM_S16LE: return OM_CODEC_PCM_S16LE;
    case AV_CODEC_ID_PCM_S32LE: return OM_CODEC_PCM_S32LE;
    case AV_CODEC_ID_AC3: return OM_CODEC_AC3;
    case AV_CODEC_ID_EAC3: return OM_CODEC_EAC3;

    default: return OM_CODEC_NONE;
  }
}

class FFmpegDemuxer final : public BaseDemuxer {
private:
  static constexpr size_t k_io_buffer_size = 65536; // 64 KiB

  AVFormatContext* fmt_ctx_ = nullptr;
  AVIOContext* avio_ctx_ = nullptr;
  uint8_t* io_buf_ = nullptr; // owned by avio_ctx_ after open

  std::thread read_thread_;
  std::mutex seek_mutex_;
  std::atomic_bool stop_reading_ {false};
  std::atomic_bool seek_pending_ {false};

public:
  FFmpegDemuxer() = default;

  auto open(std::unique_ptr<InputStream> input) -> OMError override {
    input_ = std::move(input);
    if (!input_ || !input_->isValid()) {
      return OM_IO_INVALID_STREAM;
    }

    if (!setupAvio()) {
      return OM_FORMAT_UNKNOWN_ERROR;
    }

    //fmt_ctx_ = avformat_alloc_context();
    if (!fmt_ctx_) {
      return OM_FORMAT_UNKNOWN_ERROR;
    }

    fmt_ctx_->pb = avio_ctx_;

    int ret;
    //int ret = avformat_open_input(&fmt_ctx_, nullptr, nullptr, nullptr);
    if (ret < 0) {
      // avformat_open_input frees fmt_ctx_ on failure.
      fmt_ctx_ = nullptr;
      avio_ctx_ = nullptr; // also freed by avformat
      io_buf_ = nullptr;
      return OM_FORMAT_UNKNOWN_ERROR;
    }

    //ret = avformat_find_stream_info(fmt_ctx_, nullptr);
    if (ret < 0) {
      close();
      return OM_FORMAT_UNKNOWN_ERROR;
    }

    return OM_SUCCESS;
  }

  auto readPacket() -> Result<Packet, OMError> override {
    return Err(OM_COMMON_NOT_INITIALIZED);
  }

  auto seek(int64_t timestamp_ns, int32_t stream_index) -> OMError override {
    if (!fmt_ctx_) {
      return OM_COMMON_NOT_INITIALIZED;
    }

    std::lock_guard<std::mutex> lock(seek_mutex_);

    /*int ret = av_seek_frame(fmt_ctx_, stream_index, timestamp,
                                AVSEEK_FLAG_BACKWARD);

        if (ret < 0) {
            return false;
        }*/

    seek_pending_.store(true);
    return OM_SUCCESS;
  }

private:
  auto setupAvio() -> bool {
    // av_malloc is required here — FFmpeg will free this buffer itself when
    // avio_context_free() is called.
    //io_buf_ = static_cast<uint8_t*>(av_malloc(k_io_buffer_size));
    if (!io_buf_) {
      return false;
    }

    InputStream* raw_input = input_.get();
    const bool seekable = raw_input->canSeek();

    /*avio_ctx_ = avio_alloc_context(
            io_buf_,
            static_cast<int>(k_io_buffer_size),
            0,              // write_flag = 0 (read-only)
            raw_input,      // opaque pointer passed to callbacks
            &readPacket,
            nullptr,        // no write callback
            seekable ? &seekStream : nullptr
        );*/

    if (!avio_ctx_) {
      //av_free(io_buf_);
      io_buf_ = nullptr;
      return false;
    }

    avio_ctx_->seekable = seekable ? AVIO_SEEKABLE_NORMAL : 0;
    return true;
  }
};

const FormatDescriptor FORMAT_FFMPEG = {
    .container_id = OM_CONTAINER_NONE,
    .name = "ffmpeg",
    .long_name = "FFmpeg",
    .demuxer_factory = [] { return std::make_unique<FFmpegDemuxer>(); },
    .muxer_factory = {},
};

} // namespace openmedia
