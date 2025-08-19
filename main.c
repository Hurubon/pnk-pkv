// Copyright (C) 2025 Hrvoje 'Hurubon' Žohar
// See the end of the file for extended copyright information.
#include <math.h>
#include <stdio.h>
#include <stdint.h>

#include <libavutil/frame.h>
#include <libswscale/swscale.h>

#define PNK_MEDIA_FACADE_SOURCE
#include <pnk/media_facade.h>
#include <pnk/c23_compatibility.h>

typedef struct ScalingContext {
    struct SwsContext* sws_context;
    AVFrame*           scaled_frame;
} ScalingContext;

// 24 bytes per pixel + 1 byte per row for newline + 3 bytes for \u001B[H + \0.
enum {
    RESOLUTION_W = 384,
    RESOLUTION_H = 108
};
static char frame_buffer[(RESOLUTION_W + 1) * RESOLUTION_H * 24 + 3 + 1];

static char const characters[] =
    " .`':_;^!~\"<>/\\=*?|vLxTlYzrinJjFIeVXyZSh4kUwP5KbmH9%GO#80gMBW$Q@";

PNK_MAYBE_UNUSED
static float const densities[128] = {
    ['\''] = 0.0516,
    [' '] = 0.0000,
    ['!'] = 0.0964,
    ['"'] = 0.1032,
    ['#'] = 0.2742,
    ['$'] = 0.3076,
    ['%'] = 0.2581,
    ['*'] = 0.1507,
    ['.'] = 0.0277,
    ['/'] = 0.1445,
    ['0'] = 0.2831,
    ['4'] = 0.2310,
    ['5'] = 0.2445,
    ['8'] = 0.2808,
    ['9'] = 0.2569,
    [':'] = 0.0553,
    [';'] = 0.0831,
    ['<'] = 0.1360,
    ['='] = 0.1460,
    ['>'] = 0.1388,
    ['?'] = 0.1539,
    ['@'] = 0.3310,
    ['B'] = 0.2989,
    ['F'] = 0.2016,
    ['G'] = 0.2631,
    ['H'] = 0.2548,
    ['I'] = 0.2035,
    ['J'] = 0.1935,
    ['K'] = 0.2499,
    ['L'] = 0.1627,
    ['M'] = 0.2923,
    ['O'] = 0.2678,
    ['P'] = 0.2425,
    ['Q'] = 0.3154,
    ['S'] = 0.2264,
    ['T'] = 0.1693,
    ['U'] = 0.2401,
    ['V'] = 0.2144,
    ['W'] = 0.3050,
    ['X'] = 0.2173,
    ['Y'] = 0.1761,
    ['Z'] = 0.2212,
    ['\\'] = 0.1445,
    ['^'] = 0.0866,
    ['_'] = 0.0723,
    ['`'] = 0.0476,
    ['b'] = 0.2520,
    ['e'] = 0.2121,
    ['g'] = 0.2898,
    ['h'] = 0.2278,
    ['i'] = 0.1879,
    ['j'] = 0.1995,
    ['k'] = 0.2339,
    ['l'] = 0.1707,
    ['m'] = 0.2535,
    ['n'] = 0.1921,
    ['r'] = 0.1824,
    ['v'] = 0.1609,
    ['w'] = 0.2407,
    ['x'] = 0.1674,
    ['y'] = 0.2189,
    ['z'] = 0.1811,
    ['|'] = 0.1588,
    ['~'] = 0.1015,
};

void callback(
    AVFrame* const restrict source_frame,
    void*    const restrict user_data)
{
    PnkScalingContext* const scaling_context = user_data;

    SwsContext* const sws_context  = scaling_context->sws_context;
    AVFrame*    const scaled_frame = scaling_context->scaled_frame;

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

            pointer += sprintf(pointer, "\x1B[38;2;%d;%d;%dm%c\x1B[0m",
                r, g, b, characters[lround((sizeof characters - 1) * luma)]);
        }
        *pointer++ = '\n';
    }
    
    pointer += sprintf(pointer, "\x1B[H");
    
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

    PnkMedia media = pnk_media_acquire(argv[1]);
    if (media.error < 0)
    {
        printf("Error opening '%s': %s.", argv[1], av_err2str(media.error));
        return media.error;
    }

    setvbuf(stdout, NULL, _IOFBF, 65536);

    PnkScalingContext scaling_context = pnk_media_scaling_context_acquire(
        &media, RESOLUTION_W, RESOLUTION_H, AV_PIX_FMT_RGB24, SWS_AREA);

    int const error = pnk_media_decode_and_process(
        &media, &scaling_context, &callback);
    
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
** along with this program. If not, see <https://www.gnu.org/licenses/>. */
