// Copyright (C) 2025 Hrvoje 'Hurubon' Žohar
// See the end of the file for extended copyright information.
#ifndef PNK_MEDIA_FACADE_HEADER
#define PNK_MEDIA_FACADE_HEADER

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

typedef struct PNK_Media {
    AVFormatContext* context;
    int              error;
} PNK_Media;

typedef struct PNK_Codec {
    AVCodec const*     codec;
    AVCodecContext*    context;
    int                index;
} PNK_Codec;

typedef struct PNK_DecodingContext {
    AVPacket* packet;
    AVFrame*  frame;
} PNK_DecodingContext;

typedef void (*PNK_FrameProcessor)(AVFrame*, void*);

static PNK_Media           pnk_media_acquire                  (char const*         path);
static void                pnk_media_release                  (PNK_Media           media);
static PNK_Codec           pnk_media_video_codec_acquire      (PNK_Media           media);
static void                pnk_media_video_codec_release      (PNK_Codec           codec);
static PNK_DecodingContext pnk_media_decoding_context_acquire (void);
static void                pnk_media_decoding_context_release (PNK_DecodingContext context);
static int                 pnk_media_encoded_width            (PNK_Media           media,
                                                               PNK_Codec           codec);
static int                 pnk_media_encoded_height           (PNK_Media           media,
                                                               PNK_Codec           codec);
static int                 pnk_media_decode_and_process       (PNK_Media           media,
                                                               PNK_Codec           codec,
                                                               PNK_DecodingContext context,
                                                               PNK_FrameProcessor  callback,
                                                               void*               userdata);

#endif /* PNK_MEDIA_FACADE_HEADER */

#ifdef PNK_MEDIA_FACADE_SOURCE

[[nodiscard]] static
PNK_Media
pnk_media_acquire(
    char const* const path)
{
    PNK_Media media = {0};

    media.error = avformat_open_input(&media.context, path, nullptr, nullptr);
    if (media.error < 0)
        return media;
    
    media.error = avformat_find_stream_info(media.context, nullptr);
    if (media.error < 0)
        return media;
    
    return media;
}

static
void
pnk_media_release(
    PNK_Media media)
{
    if (media.context == nullptr)
        return;
    
    avformat_close_input(&media.context);
}

[[nodiscard]] static
PNK_Codec
pnk_media_video_codec_acquire(
    PNK_Media const media)
{
    PNK_Codec codec = {};
    codec.index = av_find_best_stream(
        media.context, AVMEDIA_TYPE_VIDEO, -1, -1, &codec.codec, 0);
    codec.context = avcodec_alloc_context3(codec.codec);
    
    void* const parameters = media.context->streams[codec.index]->codecpar;
    
    avcodec_parameters_to_context(codec.context, parameters);
    return codec;
}

static
void
pnk_media_video_codec_release(
    PNK_Codec codec)
{
    if (codec.context == nullptr)
        return;
    
    avcodec_free_context(&codec.context);
}

[[nodiscard]] static
PNK_DecodingContext
pnk_media_decoding_context_acquire(void)
{
    return (PNK_DecodingContext){
        .packet = av_packet_alloc(),
        .frame  = av_frame_alloc(),
    };
}
static
void
pnk_media_decoding_context_release(
    PNK_DecodingContext context)
{
    if (context.frame != nullptr)
        av_frame_free(&context.frame);
    
    if (context.packet != nullptr)
        av_packet_free(&context.packet);
}

[[nodiscard]] static
int
pnk_media_encoded_width(
    PNK_Media const media,
    PNK_Codec const codec)
{
    AVStream*          const video_stream = media.context->streams[codec.index];
    AVCodecParameters* const parameters   = video_stream->codecpar;
    AVRational         const aspect_ratio = video_stream->sample_aspect_ratio;

    if (aspect_ratio.num == 0)
        return parameters->width;
    else
        return av_rescale(parameters->width, aspect_ratio.num, aspect_ratio.den);
}

[[nodiscard]] static
int
pnk_media_encoded_height(
    PNK_Media const media,
    PNK_Codec const codec)
{
    return media.context->streams[codec.index]->codecpar->height;
}

[[nodiscard]] static
int
pnk_media_decode_and_process(
    PNK_Media           const media,
    PNK_Codec           const codec,
    PNK_DecodingContext const context,
    PNK_FrameProcessor        callback,
    void*                     userdata)
{
    int const error = avcodec_open2(codec.context, codec.codec, nullptr);
    if (error < 0)
        return error;
    
    while (av_read_frame(media.context, context.packet) >= 0)
    {
        if (context.packet->stream_index == codec.index &&
            avcodec_send_packet(codec.context, context.packet) >= 0)
        {
            while (avcodec_receive_frame(codec.context, context.frame) >= 0)
            {
                callback(context.frame, userdata);
            }
            av_packet_unref(context.packet);
        }
    }


    return error;
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
** along with this program.  If not, see <https://www.gnu.org/licenses/>. */
