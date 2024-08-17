#ifndef PNK_MEDIA_FACADE_HEADER
#define PNK_MEDIA_FACADE_HEADER

#include <libavutil/imgutils.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

#include <pnk/c17_compatibility.h>

enum
{
    PNK_MEDIA_UNRELATED = -1,
};

typedef struct PNK_Media
{
    AVFormatContext* context;
    int              error;
    char             padding1[sizeof(AVFormatContext*) - sizeof(int)];
} PNK_Media;

typedef struct PNK_Codec
{
    AVCodec const* value;
    int            index;
    char           padding1[sizeof(AVCodec const*) - sizeof(int)];
} PNK_Codec;

typedef void(*PNK_FrameProcessor)(AVFrame*);

static PNK_Media pnk_media_acquire                (char const*   path);
static void      pnk_media_release                (PNK_Media     media);
static PNK_Codec pnk_media_find_best_video_stream (PNK_Media     media,
                                                   int           related_stream);
static PNK_Codec pnk_media_find_best_audio_stream (PNK_Media     media,
                                                   int           related_stream);
static int       pnk_media_decode_and_process     (PNK_Media     media,
                                                   PNK_Codec     codec,
                                                   PNK_FrameProcessor process);

#endif /* PNK_MEDIA_FACADE_HEADER */

#ifdef PNK_MEDIA_FACADE_SOURCE

PNK_MAYBE_UNUSED PNK_NODISCARD static
PNK_Media
pnk_media_acquire(
    char const* const path)
{
    PNK_Media media = {0};

    media.error = avformat_open_input(&media.context, path, NULL, NULL);
    if (media.error < 0)
        return media;

    media.error = avformat_find_stream_info(media.context, NULL);
    if (media.error < 0)
        return media;

    return media;
}

PNK_MAYBE_UNUSED static
void
pnk_media_release(
    PNK_Media media)
{
    if (media.context == NULL)
        return;

    avformat_close_input(&media.context);
}

PNK_MAYBE_UNUSED PNK_NODISCARD static
PNK_Codec
pnk_media_find_best_video_stream(
    PNK_Media const media,
    int            const related_stream)
{
    PNK_Codec codec = {
        .index = av_find_best_stream(
            media.context, AVMEDIA_TYPE_VIDEO, -1, related_stream,
            &codec.value, 0)
    };
    return codec;
}

PNK_MAYBE_UNUSED PNK_NODISCARD static
PNK_Codec
pnk_media_find_best_audio_stream(
    PNK_Media const media,
    int            const related_stream)
{
    PNK_Codec codec = {
        .index = av_find_best_stream(
            media.context, AVMEDIA_TYPE_AUDIO, -1, related_stream,
            &codec.value, 0)
    };
    return codec;
}

PNK_MAYBE_UNUSED PNK_NODISCARD static
int
pnk_media_decode_and_process(
    PNK_Media     media,
    PNK_Codec          codec,
    PNK_FrameProcessor process)
{
    AVCodecParameters* const parameters = media.context->streams[codec.index]->codecpar;

    AVCodecContext* codec_context = avcodec_alloc_context3(codec.value);
    avcodec_parameters_to_context(codec_context, parameters);

    int const error = avcodec_open2(codec_context, codec.value, NULL);
    if (error < 0)
        goto fail;
    
    AVPacket* packet = av_packet_alloc();
    AVFrame*  frame  = av_frame_alloc();

    while (av_read_frame(media.context, packet) >= 0)
    {
        if (packet->stream_index == codec.index &&
            avcodec_send_packet(codec_context, packet) >= 0)
        {
            while (avcodec_receive_frame(codec_context, frame) >= 0)
            {
                process(frame);
            }
            av_packet_unref(packet); 
        }
    }

    av_frame_free(&frame);
    av_packet_free(&packet);
fail:
    avcodec_free_context(&codec_context);
    return error;
}

#endif /* PNK_MEDIA_FACADE_SOURCE */
