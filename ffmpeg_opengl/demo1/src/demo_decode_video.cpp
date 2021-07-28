/*
 * Copyright (c) 2001 Fabrice Bellard
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/**
 * @file
 * video decoding with libavcodec API example
 *
 * @example decode_video.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern "C"
{
#include <libavcodec/avcodec.h>
}
#define INBUF_SIZE 4096


static void decode1(AVCodecContext *dec_ctx, AVFrame *frame, AVPacket *pkt, FILE *f)
{
    char buf[1024];
    int ret;

    ret = avcodec_send_packet(dec_ctx, pkt);
    if (ret < 0) {
        fprintf(stderr, "Error sending a packet for decoding\n");
        exit(1);
    }

    while (ret >= 0) {
        ret = avcodec_receive_frame(dec_ctx, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        else if (ret < 0) {
            fprintf(stderr, "Error during decoding\n");
            exit(1);
        }
        printf("saving frame %3d\n", dec_ctx->frame_number);
        fflush(stdout);

        int i = 0;
        uint8_t *pData = NULL;

        //write Y
        pData = frame->data[0];
        for (i = 0; i < frame->height; i++) {
            fwrite(pData, 1, frame->width, f);    //padding需要跳过，frame->width为实际宽度，frame->linesize[0]为跨度
            pData += frame->linesize[0];          //padding需要跳过，padding = frame->linesize[0] - frame->width
        }

        //write U
        pData = frame->data[1];
        for (i = 0; i < frame->height/2; i++) {
            fwrite(pData, 1, frame->width/2, f);
            pData += frame->linesize[1];
        }

        //write V
        pData = frame->data[2];
        for (i = 0; i < frame->height/2; i++) {
                fwrite(pData, 1, frame->width/2, f);
                pData += frame->linesize[2];
        }
    }
}

int demo_decode_video(const char *filename, const char *outfilename)
{
    const AVCodec *codec;
    AVCodecParserContext *parser;
    AVCodecContext *c= NULL;
    FILE *fin;
    FILE *fout;
    AVFrame *frame;
    uint8_t inbuf[INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
    uint8_t *data;
    size_t   data_size;
    int ret;
    AVPacket *pkt;


    /* set end of buffer to 0 (this ensures that no overreading happens for damaged MPEG streams) */
    memset(inbuf + INBUF_SIZE, 0, AV_INPUT_BUFFER_PADDING_SIZE);

    // AVCodec 编解码器
    codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (!codec) {
        fprintf(stderr, "Codec not found\n");
        exit(1);
    }

    // AVCodecContext 编解码器内容
    c = avcodec_alloc_context3(codec);
    if (!c) {
        fprintf(stderr, "Could not allocate video codec context\n");
        exit(1);
    }

    /* For some codecs, such as msmpeg4 and mpeg4, width and height
       MUST be initialized there because this information is not
       available in the bitstream. */

    // open AVCodec 打开编解码器
    if (avcodec_open2(c, codec, NULL) < 0) {
        fprintf(stderr, "Could not open codec\n");
        exit(1);
    }

    // AVCodecParserContext
    parser = av_parser_init(codec->id);
    if (!parser) {
        fprintf(stderr, "parser not found\n");
        exit(1);
    }

    // AVPacket 已压缩的数据
    pkt = av_packet_alloc();
    if (!pkt)
        exit(1);

    // AVFrame 未压缩的数据
    frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "Could not allocate video frame\n");
        exit(1);
    }

    fin = fopen(filename, "rb");
    if (!fin) {
        fprintf(stderr, "Could not open %s\n", filename);
        exit(1);
    }

    fout = fopen(outfilename, "wb");
    if (!fout) {
        fprintf(stderr, "Could not open %s\n", outfilename);
        exit(1);
    }

    while (!feof(fin)) {
        /* read raw data from the input file */
        data_size = fread(inbuf, 1, INBUF_SIZE, fin);
        if (!data_size)
            break;

        /* use the parser to split the data into frames */
        data = inbuf;
        while (data_size > 0) {
            ret = av_parser_parse2(parser, c, &pkt->data, &pkt->size,
                                   data, data_size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
            if (ret < 0) {
                fprintf(stderr, "Error while parsing\n");
                exit(1);
            }
            data      += ret;
            data_size -= ret;

            if (pkt->size) {
                fprintf(stdout, "pkt->size:%d.\n", pkt->size);
                decode1(c, frame, pkt, fout);
            }
        }
    }

    /* flush the decoder */
    decode1(c, frame, NULL, fout);

    fclose(fin);
    fclose(fout);

    av_parser_close(parser);
    avcodec_free_context(&c);
    av_frame_free(&frame);
    av_packet_free(&pkt);

    return 0;
}
