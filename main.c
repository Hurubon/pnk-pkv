#include <stdio.h>

#include <libswscale/swscale.h>

#define PNK_MEDIA_FACADE_SOURCE
#include <pnk/media_facade.h>

char const luma_to_ascii_lut[] = "_.,-=+:;cba!?0123456789#$W@";

void process(AVFrame* source_frame)
{
    AVFrame* scaled_frame = av_frame_alloc();

    scaled_frame->width  = 192;
    scaled_frame->height = 108;
    scaled_frame->format = source_frame->format;

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
        
    for (int row = 0; row < scaled_frame->height; ++row)
    {
        for (int col = 0; col < scaled_frame->width; ++col)
        {
            int const y = scaled_frame->data[0][row * scaled_frame->linesize[0] + 2 * col];
            printf("\e[38;2;%d;%d;%dm%c\e[0m", y, y, y, luma_to_ascii_lut[y / 27]);
        }
        puts("");
    }
    printf("\e[H");
    fflush(stdout);
    
    av_freep(&scaled_frame->data[0]);
    av_frame_free(&scaled_frame);
    sws_freeContext(sws_context);
}

int main(int argc, char* argv[])
{
    PNK_Media const media = pnk_media_acquire(argv[1]);
    PNK_Codec const codec = pnk_media_find_best_video_stream(media, PNK_MEDIA_UNRELATED);

    int const result = pnk_media_decode_and_process(media, codec, process);
    
    pnk_media_release(media);
}
