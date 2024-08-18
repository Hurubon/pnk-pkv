#include <stdio.h>

#define PNK_MEDIA_FACADE_SOURCE
#include <pnk/media_facade.h>

enum
{
    IMAGE_W = 480,
    IMAGE_H = IMAGE_W * 9 / 32,
};

char value_to_ascii_lut[] = " `.,:;'-_^=><+!rc*/z?sLTv)J7(|Fi{C}fI31tlu[neoZ5Yxjya]2ESwqkP6h9d4VpOGbUAKXHm8RD#$Bg0MNWQ%&@";

char const* byte_to_str_lut[] =
{
     "16m",  "17m",  "18m",  "19m",  "20m",  "21m",  "22m",  "23m",
     "24m",  "25m",  "26m",  "27m",  "28m",  "29m",  "30m",  "31m",
     "32m",  "33m",  "34m",  "35m",  "36m",  "37m",  "38m",  "39m",
     "40m",  "41m",  "42m",  "43m",  "44m",  "45m",  "46m",  "47m",
     "48m",  "49m",  "50m",  "51m",  "52m",  "53m",  "54m",  "55m",
     "56m",  "57m",  "58m",  "59m",  "60m",  "61m",  "62m",  "63m",
     "64m",  "65m",  "66m",  "67m",  "68m",  "69m",  "70m",  "71m",
     "72m",  "73m",  "74m",  "75m",  "76m",  "77m",  "78m",  "79m",
     "80m",  "81m",  "82m",  "83m",  "84m",  "85m",  "86m",  "87m",
     "88m",  "89m",  "90m",  "91m",  "92m",  "93m",  "94m",  "95m",
     "96m",  "97m",  "98m",  "99m", "100m", "101m", "102m", "103m",
    "104m", "105m", "106m", "107m", "108m", "109m", "110m", "111m",
    "112m", "113m", "114m", "115m", "116m", "117m", "118m", "119m",
    "120m", "121m", "122m", "123m", "124m", "125m", "126m", "127m",
    "128m", "129m", "130m", "131m", "132m", "133m", "134m", "135m",
    "136m", "137m", "138m", "139m", "140m", "141m", "142m", "143m",
    "144m", "145m", "146m", "147m", "148m", "149m", "150m", "151m",
    "152m", "153m", "154m", "155m", "156m", "157m", "158m", "159m",
    "160m", "161m", "162m", "163m", "164m", "165m", "166m", "167m",
    "168m", "169m", "170m", "171m", "172m", "173m", "174m", "175m",
    "176m", "177m", "178m", "179m", "180m", "181m", "182m", "183m",
    "184m", "185m", "186m", "187m", "188m", "189m", "190m", "191m",
    "192m", "193m", "194m", "195m", "196m", "197m", "198m", "199m",
    "200m", "201m", "202m", "203m", "204m", "205m", "206m", "207m",
    "208m", "209m", "210m", "211m", "212m", "213m", "214m", "215m",
    "216m", "217m", "218m", "219m", "220m", "230m", "240m", "241m",
    "242m", "243m", "244m", "245m", "246m", "247m", "248m", "249m",
};

/* 16 bytes per pixel + 1 byte per row for newline + 3 bytes for \e[H + \0 */
_Alignas(4096) char frame_buffer[IMAGE_W * IMAGE_H * 16 + IMAGE_H + 4];

static char* stream_write(char* restrict dst, char const* restrict src, size_t bytes)
{
    return memcpy(dst, src, bytes) + bytes;
}

/* Map a value in the range [0, old_max] to [0, new_max]. */
static int map0(int v, int old_max, int new_max)
{
    return v * new_max / old_max;
}

static void process(AVFrame* source_frame)
{
    AVFrame* scaled_frame = av_frame_alloc();

    scaled_frame->width  = IMAGE_W;
    scaled_frame->height = IMAGE_H;
    scaled_frame->format = AV_PIX_FMT_RGB24;

    struct SwsContext* sws_context = sws_getContext(
        source_frame->width, source_frame->height, source_frame->format,
        scaled_frame->width, scaled_frame->height, scaled_frame->format,
        SWS_LANCZOS, NULL, NULL, NULL);

    av_image_alloc(
        scaled_frame->data, scaled_frame->linesize,
        scaled_frame->width, scaled_frame->height,
        scaled_frame->format, 4096);
    
    sws_scale(sws_context,
        (uint8_t const* const*)
        source_frame->data, source_frame->linesize, 0, source_frame->height,
        scaled_frame->data, scaled_frame->linesize);

    char* pointer = frame_buffer;
        
    for (int row = 0; row < scaled_frame->height; ++row)
    {
        for (int col = 0; col < scaled_frame->width; ++col)
        {
            int const r = scaled_frame->data[0][row * scaled_frame->linesize[0] + 3 * col + 0];
            int const g = scaled_frame->data[0][row * scaled_frame->linesize[0] + 3 * col + 1];
            int const b = scaled_frame->data[0][row * scaled_frame->linesize[0] + 3 * col + 2];

            int const color_code = 36 * map0(r, 255, 5) + 6 * map0(g, 255, 5) + map0(b, 255, 5);

            pointer = stream_write(pointer, "\x1B[38;5;", 7);
            pointer = stream_write(pointer, byte_to_str_lut[color_code], 3 + (color_code >= 84));
            *pointer++ = value_to_ascii_lut[map0((r + g + b) / 3, 255, 92)];
            pointer = stream_write(pointer, "\x1B[0m", 4);
        }
        *pointer++ = '\n';
    }
    pointer = stream_write(pointer, "\x1B[H\0", 4);

    fwrite(frame_buffer, 1, pointer - frame_buffer, stdout);
    fflush(stdout);

    av_freep(&scaled_frame->data[0]);
    av_frame_free(&scaled_frame);
    sws_freeContext(sws_context);
}

int main(int argc, char* argv[])
{
    setvbuf(stdout, NULL, _IOFBF, 1ull << 17);

    PNK_Media const media = pnk_media_acquire(argv[1]);
    PNK_Codec const codec = pnk_media_find_best_video_stream(media, PNK_MEDIA_UNRELATED);

    printf("\x1B[?25l");

    int const result = pnk_media_decode_and_process(media, codec, process);
    
    pnk_media_release(media);
}
