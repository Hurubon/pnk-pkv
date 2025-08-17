// Copyright (C) 2025 Hrvoje 'Hurubon' Žohar
// See the end of the file for extended copyright information.
#include <math.h>
#include <stdio.h>
#include <stdint.h>

#include <libavutil/frame.h>
#include <libswscale/swscale.h>

#define PNK_MEDIA_FACADE_SOURCE
#include <pnk/media_facade.h>

typedef struct ScalingContext {
    struct SwsContext* sws_context;
    AVFrame*           scaled_frame;
} ScalingContext;


// 24 bytes per pixel + 1 byte per row for newline + 3 bytes for \u001B[H + \0.
constexpr int RESOLUTION_W = 384;
constexpr int RESOLUTION_H = 108;
char frame_buffer[(RESOLUTION_W + 1) * RESOLUTION_H * 24 + 4];

constexpr char lookup[] = " .`:_;!\"</*|xYr{jeySkUK6GD8&BWQ@";

void callback(
    AVFrame* const source_frame,
    void*    const user_data)
{
    PNK_ScalingContext* const scaling_context = user_data;

    auto const sws_context  = scaling_context->sws_context;
    auto const scaled_frame = scaling_context->scaled_frame;

    sws_scale(sws_context,
        (uint8_t const* const*)
        source_frame->data, source_frame->linesize, 0, source_frame->height,
        scaled_frame->data, scaled_frame->linesize);

    char* pointer = frame_buffer;
        
    for (int row = 0; row < scaled_frame->height; row += 1)
    {
        for (int col = 0; col < scaled_frame->width; col += 1)
        {
            int const r = scaled_frame
                ->data[0][row * scaled_frame->linesize[0] + 3 * col + 0];
            int const g = scaled_frame
                ->data[0][row * scaled_frame->linesize[0] + 3 * col + 1];
            int const b = scaled_frame
                ->data[0][row * scaled_frame->linesize[0] + 3 * col + 2];

            // https://en.wikipedia.org/wiki/Rec._709
            float const luma = lround(0.2126*r + 0.7152*g + 0.0722*b) / 255.0f;

            auto const character_index = lround((sizeof lookup - 1) * luma);

            pointer += sprintf(pointer, "\u001B[38;2;%d;%d;%dm%c\u001B[0m",
                r, g, b, lookup[character_index]);
        }
        *pointer++ = '\n';
    }
    pointer += sprintf(pointer, "\u001b[H");
    
    fwrite(frame_buffer, 1, pointer - frame_buffer, stdout);
    fflush(stdout);
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        printf("Usage: pkv <path>.");
        return EXIT_FAILURE;
    }

    PNK_Media media = pnk_media_acquire(argv[1]);
    if (media.error < 0)
    {
        printf("Error opening '%s': %s.", argv[1], av_err2str(media.error));
        return media.error;
    }

    setvbuf(stdout, NULL, _IOFBF, 65536);

    PNK_ScalingContext scaling_context = pnk_media_scaling_context_acquire(
        &media, RESOLUTION_W, RESOLUTION_H, AV_PIX_FMT_RGB24, SWS_AREA);

    int const error = pnk_media_decode_and_process(
        &media, &callback, &scaling_context);
    
    if (error < 0)
    {
        printf("Error: %s.", av_err2str(error));
    }

    pnk_media_scaling_context_release(scaling_context);
    pnk_media_release(&media);
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
