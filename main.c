// Copyright (C) 2025 Hrvoje 'Hurubon' Žohar
// See the end of the file for extended copyright information.
#include <stdio.h>
#include <math.h>

#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <libavformat/avformat.h>

#define PNK_MEDIA_FACADE_SOURCE
#include <pnk/media_facade.h>

typedef struct ScalingContext {
    struct SwsContext* sws_context;
    AVFrame*           scaled_frame;
} ScalingContext;

enum
{
    PLAYBACK_RESOLUTION_W = 384,
    PLAYBACK_RESOLUTION_H = 108,
};

static int clamp(
    long const value,
    long const min,
    long const max)
{
    return value >= min
        ? value <= max
            ? value
            : max
        : min;
}

// 24 bytes per pixel + 1 byte per row for newline + 3 bytes for \e[H + \0
char frame_buffer[PLAYBACK_RESOLUTION_W * PLAYBACK_RESOLUTION_H * 24 + 108 + 4];

char const lookup[] = " .`:_;!\"</*|xYr{jeySkUK6GD8&BWQ@";

void callback(
    AVFrame* const source_frame,
    void   * const userdata)
{
    ScalingContext* const scaling_context = userdata;
    AVFrame*        const scaled_frame    = scaling_context->scaled_frame;

    sws_scale(scaling_context->sws_context,
        (uint8_t const* const*)
        source_frame->data, source_frame->linesize, 0, source_frame->height,
        scaled_frame->data, scaled_frame->linesize);

    char* pointer = frame_buffer;
        
    for (int row = 0; row < scaled_frame->height; row += 1)
    {
        for (int col = 0; col < scaled_frame->width; col += 1)
        {
            long const r = scaled_frame->data[0][row * scaled_frame->linesize[0] + 3 * col + 0];
            long const g = scaled_frame->data[0][row * scaled_frame->linesize[0] + 3 * col + 1];
            long const b = scaled_frame->data[0][row * scaled_frame->linesize[0] + 3 * col + 2];
            long const y = lround(0.2126 * r + 0.7152 * g + 0.0722 * b);

            long const character_index = lround(y / 255.0 * (sizeof lookup - 1));

            // This exponent was approximated numerically.
            double const dimming_factor = pow(y / 255.0, 0.7056696172415708);
            double const dimming_adjust = dimming_factor == 0.0? 1.0 : 1.0 / dimming_factor;
            
            int const r_corrected = clamp(r * dimming_adjust, 0, 255);
            int const g_corrected = clamp(g * dimming_adjust, 0, 255);
            int const b_corrected = clamp(b * dimming_adjust, 0, 255);

            pointer += sprintf(pointer, "\e[38;2;%d;%d;%dm%c\e[0m",
                r_corrected, g_corrected, b_corrected, lookup[character_index]);
        }
        *pointer++ = '\n';
    }
    pointer += sprintf(pointer, "\e[H");
    
    fwrite(frame_buffer, 1, pointer - frame_buffer, stdout);
    fflush(stdout);
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: pkv <path>.");
        return -1;
    }

    PNK_Media media = pnk_media_acquire(argv[1]);
    if (media.error < 0)
    {
        fprintf(stderr, "Error opening '%s': %s.", argv[1], av_err2str(media.error));
        return media.error;
    }
    
    PNK_Codec codec = pnk_media_video_codec_acquire(media);
    PNK_DecodingContext decoding_context = pnk_media_decoding_context_acquire();

    AVFrame* scaled_frame = av_frame_alloc();
    scaled_frame->width  = PLAYBACK_RESOLUTION_W;
    scaled_frame->height = PLAYBACK_RESOLUTION_H;
    scaled_frame->format = AV_PIX_FMT_RGB24;

    av_image_alloc(
        scaled_frame->data  , scaled_frame->linesize,
        scaled_frame->width , scaled_frame->height,
        scaled_frame->format, 4096);

    int const source_frame_width  = pnk_media_encoded_width (media, codec);
    int const source_frame_height = pnk_media_encoded_height(media, codec);
    int const source_frame_format = codec.context->pix_fmt;

    struct SwsContext* sws_context = sws_getContext(
        source_frame_width , source_frame_height , source_frame_format ,
        scaled_frame->width, scaled_frame->height, scaled_frame->format,
        SWS_AREA, nullptr, nullptr, nullptr);

    ScalingContext userdata = { sws_context, scaled_frame };

    int const result = pnk_media_decode_and_process(
        media, codec, decoding_context,
        callback, &userdata);

    sws_freeContext(sws_context);
    av_freep(&scaled_frame->data[0]);
    av_frame_free(&scaled_frame);
    pnk_media_decoding_context_release(decoding_context);
    pnk_media_video_codec_release(codec);
    pnk_media_release(media);
}
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
