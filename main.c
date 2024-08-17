#include <stdio.h>

#include <libswscale/swscale.h>

#define PNK_MEDIA_FACADE_SOURCE
#include <pnk/media_facade.h>

enum
{
    IMAGE_W = 192 * 2,
    IMAGE_H = 108,
};

char const luma_to_ascii_lut[] = "_.,-=+:;cba!?0123456789#$W@";

char const* lookup[] =
{
    "0;",
    "1;",
    "2;",
    "3;",
    "4;",
    "5;",
    "6;",
    "7;",
    "8;",
    "9;",
    "10;",
    "11;",
    "12;",
    "13;",
    "14;",
    "15;",
    "16;",
    "17;",
    "18;",
    "19;",
    "20;",
    "21;",
    "22;",
    "23;",
    "24;",
    "25;",
    "26;",
    "27;",
    "28;",
    "29;",
    "30;",
    "31;",
    "32;",
    "33;",
    "34;",
    "35;",
    "36;",
    "37;",
    "38;",
    "39;",
    "40;",
    "41;",
    "42;",
    "43;",
    "44;",
    "45;",
    "46;",
    "47;",
    "48;",
    "49;",
    "50;",
    "51;",
    "52;",
    "53;",
    "54;",
    "55;",
    "56;",
    "57;",
    "58;",
    "59;",
    "60;",
    "61;",
    "62;",
    "63;",
    "64;",
    "65;",
    "66;",
    "67;",
    "68;",
    "69;",
    "70;",
    "71;",
    "72;",
    "73;",
    "74;",
    "75;",
    "76;",
    "77;",
    "78;",
    "79;",
    "80;",
    "81;",
    "82;",
    "83;",
    "84;",
    "85;",
    "86;",
    "87;",
    "88;",
    "89;",
    "90;",
    "91;",
    "92;",
    "93;",
    "94;",
    "95;",
    "96;",
    "97;",
    "98;",
    "99;",
    "100;",
    "101;",
    "102;",
    "103;",
    "104;",
    "105;",
    "106;",
    "107;",
    "108;",
    "109;",
    "110;",
    "111;",
    "112;",
    "113;",
    "114;",
    "115;",
    "116;",
    "117;",
    "118;",
    "119;",
    "120;",
    "121;",
    "122;",
    "123;",
    "124;",
    "125;",
    "126;",
    "127;",
    "128;",
    "129;",
    "130;",
    "131;",
    "132;",
    "133;",
    "134;",
    "135;",
    "136;",
    "137;",
    "138;",
    "139;",
    "140;",
    "141;",
    "142;",
    "143;",
    "144;",
    "145;",
    "146;",
    "147;",
    "148;",
    "149;",
    "150;",
    "151;",
    "152;",
    "153;",
    "154;",
    "155;",
    "156;",
    "157;",
    "158;",
    "159;",
    "160;",
    "161;",
    "162;",
    "163;",
    "164;",
    "165;",
    "166;",
    "167;",
    "168;",
    "169;",
    "170;",
    "171;",
    "172;",
    "173;",
    "174;",
    "175;",
    "176;",
    "177;",
    "178;",
    "179;",
    "180;",
    "181;",
    "182;",
    "183;",
    "184;",
    "185;",
    "186;",
    "187;",
    "188;",
    "189;",
    "190;",
    "191;",
    "192;",
    "193;",
    "194;",
    "195;",
    "196;",
    "197;",
    "198;",
    "199;",
    "200;",
    "201;",
    "202;",
    "203;",
    "204;",
    "205;",
    "206;",
    "207;",
    "208;",
    "209;",
    "210;",
    "211;",
    "212;",
    "213;",
    "214;",
    "215;",
    "216;",
    "217;",
    "218;",
    "219;",
    "220;",
    "221;",
    "222;",
    "223;",
    "224;",
    "225;",
    "226;",
    "227;",
    "228;",
    "229;",
    "230;",
    "231;",
    "232;",
    "233;",
    "234;",
    "235;",
    "236;",
    "237;",
    "238;",
    "239;",
    "240;",
    "241;",
    "242;",
    "243;",
    "244;",
    "245;",
    "246;",
    "247;",
    "248;",
    "249;",
    "250;",
    "251;",
    "252;",
    "253;",
    "254;",
    "255;",
};

/* 24 bytes per pixel + 1 byte per row for newline + 3 bytes for \e[H + \0 */
char frame_buffer[IMAGE_W * IMAGE_H * 24 + 108 + 4];

void process(AVFrame* source_frame)
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

            memcpy(pointer, "\e[38;2;", 7);
            pointer += 7;
            memcpy(pointer, lookup[r], 2 + (r > 9) + (r > 99));
            pointer += 2 + (r > 9) + (r > 99);
            memcpy(pointer, lookup[g], 2 + (g > 9) + (g > 99));
            pointer += 2 + (g > 9) + (g > 99);
            memcpy(pointer, lookup[b], 2 + (b > 9) + (b > 99));
            pointer += 2 + (b > 9) + (b > 99);

            pointer[-1] = 'm';

            memcpy(pointer, "$\e[0m", 5);
            pointer += 5;
        }
        *pointer++ = '\n';
    }
    memcpy(pointer, "\e[H\0", 4);
    pointer += 4;
    fwrite(frame_buffer, 1, pointer - frame_buffer, stdout);
    fflush(stdout);
    
    av_freep(&scaled_frame->data[0]);
    av_frame_free(&scaled_frame);
    sws_freeContext(sws_context);
}

int main(int argc, char* argv[])
{
    setvbuf(stdout, NULL, _IOFBF, sizeof frame_buffer);

    PNK_Media const media = pnk_media_acquire(argv[1]);
    PNK_Codec const codec = pnk_media_find_best_video_stream(media, PNK_MEDIA_UNRELATED);

    int const result = pnk_media_decode_and_process(media, codec, process);
    
    pnk_media_release(media);
}
