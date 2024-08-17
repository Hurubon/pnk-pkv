#include <stdio.h>

#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

int main(int argc, char* argv[])
{
    AVFormatContext* context = NULL;

    (void)avformat_open_input(&context, argv[1], NULL, NULL);
    (void)avformat_find_stream_info(context, NULL);

    AVCodec const* codec = NULL;

    int const index = av_find_best_stream(context, AVMEDIA_TYPE_VIDEO, -1, -1, &codec, 0);

    AVCodecParameters* const parameters = context->streams[index]->codecpar;
    AVCodecContext* codec_context = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(codec_context, parameters);

    (void)avcodec_open2(codec_context, codec, NULL);

    AVPacket* packet = av_packet_alloc();
    AVFrame*  frame  = av_frame_alloc();

    while (av_read_frame(context, packet) >= 0)
    {
        if (packet->stream_index == index &&
            avcodec_send_packet(codec_context, packet) >= 0)
        {
            while (avcodec_receive_frame(codec_context, frame) >= 0)
            {
                AVFrame* scale = av_frame_alloc();

                scale->width  = 192;
                scale->height = 108;
                scale->format = AV_PIX_FMT_RGB24;

                struct SwsContext* sws_context = sws_getContext(
                    frame->width, frame->height, frame->format,
                    scale->width, scale->height, scale->format,
                    SWS_FAST_BILINEAR, NULL, NULL, NULL);

                av_image_alloc(
                    scale->data, scale->linesize, scale->width, scale->height, scale->format, 32);
                
                sws_scale(sws_context,
                    (uint8_t const* const*)frame->data, frame->linesize, 0, frame->height,
                    scale->data, scale->linesize);
                    
                for (int row = 0; row < scale->height; ++row)
                {
                    for (int col = 0; col < scale->width; ++col)
                    {
                        int const r = scale->data[0][row * scale->linesize[0] + 3 * col + 0];
                        int const g = scale->data[0][row * scale->linesize[0] + 3 * col + 1];
                        int const b = scale->data[0][row * scale->linesize[0] + 3 * col + 2];
                        printf("\x1B[38;2;%d;%d;%dm$\x1B[0m", r, g, b);
                    }
                    puts("");
                }
                printf("\x1B[H");
                fflush(stdout);
                
                av_freep(&scale->data[0]);
                av_frame_free(&scale);
                sws_freeContext(sws_context);
            }
            av_packet_unref(packet);
        }
    }

    av_frame_free(&frame);
    av_packet_free(&packet);
    avcodec_free_context(&codec_context);
    avformat_close_input(&context);
}