// Copyright (C) 2025 Hrvoje 'Hurubon' Žohar
// See the end of the file for extended copyright information.
#ifndef PNK_MEDIA_FACADE_HEADER
#define PNK_MEDIA_FACADE_HEADER

#include <stdint.h>

#include <libavutil/frame.h>
#include <libavutil/pixfmt.h>
#include <libavutil/imgutils.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

typedef struct PnkMedia {
    AVFormatContext* context;
    AVCodecContext*  video_codec_context;
    AVCodecContext*  audio_codec_context;
    AVCodec const*   video_codec;
    AVCodec const*   audio_codec;
    int16_t          video_codec_index;
    int16_t          audio_codec_index;
    int32_t          error;
} PnkMedia;

typedef struct PnkScalingContext {
    SwsContext* sws_context;
    AVFrame*    scaled_frame;
} PnkScalingContext;

typedef void (*PnkFrameProcessor)(AVFrame* restrict, void* restrict);

static PnkMedia          pnk_media_acquire                 (char const*                 path);
static void              pnk_media_release                 (PnkMedia*                   media);
static PnkScalingContext pnk_media_scaling_context_acquire (PnkMedia const*             media,
                                                            int                         width,
                                                            int                         height,
                                                            enum AVPixelFormat          format,
                                                            int                         flags);
static void               pnk_media_scaling_context_release (PnkScalingContext          scaling_context);
static int                pnk_media_decode_and_process      (PnkMedia const*   restrict media,
                                                             void*             restrict userdata,
                                                             PnkFrameProcessor          callback);

#endif /* PNK_MEDIA_FACADE_HEADER */

#ifdef PNK_MEDIA_FACADE_SOURCE

[[nodiscard]] static
PnkMedia
pnk_media_acquire(
    char const* const path)
{
    PnkMedia media = {0};

    media.error = avformat_open_input(&media.context, path, nullptr, nullptr);
    if (media.error < 0)
    {
        return media;
    }
    
    media.error = avformat_find_stream_info(media.context, nullptr);
    if (media.error < 0)
    {
        return media;
    }

    media.video_codec_index = av_find_best_stream(
        media.context, AVMEDIA_TYPE_VIDEO, -1, -1, &media.video_codec, 0);
    if (media.video_codec_index < 0)
    {
        media.error = media.video_codec_index;
        return media;
    }

    media.audio_codec_index = av_find_best_stream(
        media.context, AVMEDIA_TYPE_AUDIO, -1, -1, &media.audio_codec, 0);
    if (media.audio_codec_index < 0)
    {
        media.error = media.video_codec_index;
        return media;
    }

    void* const video_parameters = media.context
        ->streams[media.video_codec_index]->codecpar;
    void* const audio_parameters = media.context
        ->streams[media.audio_codec_index]->codecpar;

    media.video_codec_context = avcodec_alloc_context3(media.video_codec);
    media.audio_codec_context = avcodec_alloc_context3(media.audio_codec);

    avcodec_parameters_to_context(media.video_codec_context, video_parameters);
    avcodec_parameters_to_context(media.audio_codec_context, audio_parameters);

    return media;
}

static
void
pnk_media_release(
    PnkMedia* const media)
{
    avformat_close_input(&media->context);
    avcodec_free_context(&media->video_codec_context);
    avcodec_free_context(&media->audio_codec_context);
}

[[nodiscard]] static
PnkScalingContext
pnk_media_scaling_context_acquire(
    PnkMedia const*    const media,
    int                const width,
    int                const height,
    enum AVPixelFormat const format,
    int                const flags)
{
    AVFrame* scaled_frame = av_frame_alloc();
    scaled_frame->width   = width;
    scaled_frame->height  = height;
    scaled_frame->format  = format;

    av_image_alloc(
        scaled_frame->data, scaled_frame->linesize,
        width, height, format, 1024);

    struct FrameData {
        int                width;
        int                height;
        enum AVPixelFormat format;
    };
    
    int const index = media->video_codec_index;
    struct FrameData* source_frame = &(struct FrameData){
        .width  = media->context->streams[index]->codecpar->width,
        .height = media->context->streams[index]->codecpar->height,
        .format = media->video_codec_context->pix_fmt
    };

    SwsContext* sws_context = sws_getContext(
        source_frame->width, source_frame->height, source_frame->format,
        scaled_frame->width, scaled_frame->height, scaled_frame->format,
        flags, nullptr, nullptr, nullptr);
    
    return (PnkScalingContext){
        .sws_context  = sws_context,
        .scaled_frame = scaled_frame
    };
}

static
void
pnk_media_scaling_context_release(
    PnkScalingContext scaling_context)
{
    sws_freeContext(scaling_context.sws_context);
    av_freep(&scaling_context.scaled_frame->data[0]);
    av_frame_free(&scaling_context.scaled_frame);
}

[[nodiscard]] static
int
pnk_media_decode_and_process(
    PnkMedia const*   const restrict media,
    void*             const restrict userdata,
    PnkFrameProcessor                callback)
{
    int const video_error = avcodec_open2(
        media->video_codec_context, media->video_codec, nullptr);
    if (video_error < 0)
    {
        return video_error;
    }
    int const audio_error = avcodec_open2(
        media->audio_codec_context, media->audio_codec, nullptr);
    if (audio_error < 0)
    {
        return audio_error;
    }

    AVPacket* packet = av_packet_alloc();
    AVFrame*  frame  = av_frame_alloc(); 
    
    while (av_read_frame(media->context, packet) >= 0)
    {
        if (packet->stream_index == media->video_codec_index &&
            avcodec_send_packet(media->video_codec_context, packet) >= 0)
        {
            while (avcodec_receive_frame(media->video_codec_context, frame) >= 0)
            {
                callback(frame, userdata);
                // FIXME:
                // I realized I might need this to not leak memory?
                // I feel like I'm missing this in other places as well?
                av_frame_unref(frame);
            }
            av_packet_unref(packet);
        }
    }
    
    av_frame_free(&frame);
    av_packet_free(&packet);

    return 0;
}

#endif /* PNK_MEDIA_FACADE_SOURCE */
/* pnk-pkv - a terminal ASCII video player
** Copyright (C) 2025 Hrvoje 'Hurubon' Žohar
**
** pnk-pkv is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** pnk-pkv is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program. If not, see <https://www.gnu.org/licenses/>. */
