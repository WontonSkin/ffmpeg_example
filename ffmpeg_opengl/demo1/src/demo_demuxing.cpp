/**
 * @file
 * Demuxing example.
 *
 * Show how to use the libavformat and libavcodec API to demux and
 * decode audio and video data.
 * @example demuxing_decoding.c
 */
extern "C"
{
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
}

int demo_demuxing(const char *src_filename, const char *video_dst_filename, const char *audio_dst_filename)
{
    int ret = 0;
    AVFormatContext *fmt_ctx = NULL;
    AVStream *video_stream = NULL, *audio_stream = NULL;
    FILE *video_dst_file = NULL;
    FILE *audio_dst_file = NULL;
    int video_stream_idx = -1, audio_stream_idx = -1;
    AVPacket pkt;

    //打开输入文件，并分配 AVFormatContext
    if (avformat_open_input(&fmt_ctx, src_filename, NULL, NULL) < 0) {
        fprintf(stderr, "Could not open source file %s\n", src_filename);
        exit(1);
    }

    //检索获取流信息
    if (avformat_find_stream_info(fmt_ctx, NULL) < 0) {
        fprintf(stderr, "Could not find stream information\n");
        exit(1);
    }

    //在文件中找到“最佳”流，即最匹配的流。非负数表示成功，返回此流id。
    ret = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if (ret < 0) {
        fprintf(stderr, "Could not find %s stream in input file '%s'\n",
                av_get_media_type_string(AVMEDIA_TYPE_VIDEO), src_filename);
        exit(1);
    }
    video_stream_idx = ret;
    printf("video_stream_idx:%d.\n", video_stream_idx);
    
    ret = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
    if (ret < 0) {
        fprintf(stderr, "Could not find %s stream in input file '%s'\n",
                av_get_media_type_string(AVMEDIA_TYPE_AUDIO), src_filename);
        exit(1);
    }
    audio_stream_idx = ret;
    printf("audio_stream_idx:%d.\n", audio_stream_idx);
    

    //视频 AVStream
    video_stream = fmt_ctx->streams[video_stream_idx];
    video_dst_file = fopen(video_dst_filename, "wb");
    if (!video_dst_file) {
        fprintf(stderr, "Could not open destination file %s\n", video_dst_filename);
        ret = 1;
        goto end;
    }

    //音频 AVStream
    audio_stream = fmt_ctx->streams[audio_stream_idx];
    audio_dst_file = fopen(audio_dst_filename, "wb");
    if (!audio_dst_file) {
        fprintf(stderr, "Could not open destination file %s\n", audio_dst_filename);
        ret = 1;
        goto end;
    }

    //打印有关媒体格式的详细信息，例如持续时间，比特率，流，容器，程序，元数据，辅助数据，编解码器和时基等。
    av_dump_format(fmt_ctx, 0, src_filename, 0);

    if (!audio_stream && !video_stream) {
        fprintf(stderr, "Could not find audio or video stream in the input, aborting\n");
        ret = 1;
        goto end;
    }

    //初始化 AVPacket
    av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;

    if (video_stream)
        printf("Demuxing video from file '%s' into '%s'\n", src_filename, video_dst_filename);
    if (audio_stream)
        printf("Demuxing audio from file '%s' into '%s'\n", src_filename, audio_dst_filename);

    //从文件中读取数据帧 AVPacket
    while (av_read_frame(fmt_ctx, &pkt) >= 0) {
        
        printf("stream_index:%d, data:%p, size:%d, dts:%ld, duration:%ld, pos:%ld.\n", 
                pkt.stream_index, pkt.data, pkt.size, pkt.dts, pkt.duration, pkt.pos);
        
        if (pkt.stream_index == video_stream_idx) {
            fwrite(pkt.data, 1, pkt.size, video_dst_file);
        }

        if (pkt.stream_index == audio_stream_idx) {
            fwrite(pkt.data, 1, pkt.size, audio_dst_file);
        }

        av_packet_unref(&pkt);
    }
    printf("Demuxing succeeded.\n");

end:
    if (fmt_ctx)
        avformat_close_input(&fmt_ctx);
    if (video_dst_file)
        fclose(video_dst_file);
    if (audio_dst_file)
        fclose(audio_dst_file);

    return ret;
}
