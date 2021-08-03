#include <iostream>
#include <string>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "demo_decoding.h"
#include "demo_log.h"

namespace DEMO
{

#define clip(var) ((var>=255)?255:(var<=0)?0:var)

void saveH26X(AVStream *av, AVPacket *pkt)
{   
    std::string filename;
    if (AV_CODEC_ID_H264 == av->codecpar->codec_id) {
        filename = std::string("raw.h264");
    } else if (AV_CODEC_ID_HEVC == av->codecpar->codec_id) {
        filename = std::string("raw.h265");
    } else {
        DemoLog(LOG_LEVEL_ERROR, "unknow video codec_id:%d .\n", av->codecpar->codec_id);
        return;
    }

    FILE *video_dst_file = fopen(filename.c_str(), "ab+");
    if (video_dst_file) {
        fwrite(pkt->data, 1, pkt->size, video_dst_file);
        fclose(video_dst_file);
    } else {
        DemoLog(LOG_LEVEL_ERROR, "fopen h26X fail.\n");
    }
    
    return;
}

void saveYuv(AVFrame *frame)
{
    FILE *f = fopen("raw.yuv", "ab+");
    if (NULL == f) {
        DemoLog(LOG_LEVEL_ERROR, "fopen yuv fail.\n");
        return;
    }

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
    
    fclose(f);
    
    return;
}

AV_FRAME_DATA_PTR yuv2rgb(enum AVPixelFormat pix_fmt, AVFrame *frame)
{
    //1.小数形式,未量化
    //R = Y + 1.4075 * (V-128);  
    //G = Y - 0.3455 * (U-128) - 0.7169*(V-128);  
    //B = Y + 1.779 * (U-128); 

    //2.整数形式（减少计算量）未量化
    //R= Y + ((360 * (V - 128))>>8); 
    //G= Y - (( ( 88 * (U - 128)  + 184 * (V - 128)) )>>8); 
    //B= Y +((455 * (U - 128))>>8);

    //padding需要跳过，frame->width为实际宽度，frame->linesize[0]为跨度
    //padding需要跳过，padding = frame->linesize[0] - frame->width

    //printf("pix_fmt is :%d.\n", pix_fmt);  // AV_PIX_FMT_YUV420P

    

    int j, i;
    int c, d, e;
    
    int width = frame->width;
    int height = frame->height;
    unsigned char* y = frame->data[0];
    unsigned char* u = frame->data[1];
    unsigned char* v = frame->data[2];


    //申请内存
    unsigned int yuvSize = width*height*4;
    AV_FRAME_DATA_PTR p = std::make_shared<AvFrameData>(yuvSize);
    AvDataFormat* pInfo = p->getPtr();
    pInfo->type = AV_CODEC_VIDEO;
    pInfo->vf.width = width;
    pInfo->vf.height = height;
    pInfo->frameLen = yuvSize;
    
    unsigned char* rgba = pInfo->frameBuf;
    unsigned char* line = rgba;
    
    if (AV_PIX_FMT_YUV420P == pix_fmt) {
                
        for( j = 0 ; j < height ; j++ ) {
            
            unsigned char* cur = line;

            for( i = 0 ; i < width ; i++ ) {
                
                c = y[i] - 16;
                d = u[(i>>1)] - 128;
                e = v[(i>>1)] - 128;
                
                (*cur) = clip(( 298 * c           + 409 * e + 128) >> 8); cur++; //r
                (*cur) = clip(( 298 * c - 100 * d - 208 * e + 128) >> 8); cur++; //g
                (*cur) = clip(( 298 * c + 516 * d           + 128) >> 8); cur++; //b
                (*cur) = 255;                                             cur++; //a
            }
            
            line += width << 2;
            y += frame->linesize[0];
            if ((j%2) == 1) {
                u += frame->linesize[1];
                v += frame->linesize[2];
            }
        }
    }
    
    return p;
}


DecodeObj::DecodeObj(AvDataQueue* pDateQue, std::string url) : m_stop(false), m_url(url), m_pDateQue(pDateQue)
{    
    m_pFormatCtx = NULL;
    m_pVideoStream = NULL; 
    m_pAudioStream = NULL;
    m_videoStreamIdx = -1, 
    m_audioStreamIdx = -1;
    m_pPkt = NULL;
    m_pFrame = NULL; 

    m_pCodec = NULL;
    m_pParserCtx = NULL;
    m_pCodecCtx = NULL;    
}

DecodeObj::~DecodeObj()
{

    if (m_stop != true) {
        m_stop = true;
        if (m_decodingThr.get() != nullptr) {
            DemoLog(LOG_LEVEL_INFO, "stop decodingThread.\n");
            m_decodingThr->join();
        }
    }
    
    uninitDecoding();
    
    uninitDemuxing();

    DemoLog(LOG_LEVEL_INFO, "~DecodeObj suc.\n");
}

int DecodeObj::init()
{
    int ret = 0;
    
    ret = initDemuxing(m_url.c_str());
    if (ret != 0) {
        DemoLog(LOG_LEVEL_ERROR, "initDemuxing fail.\n");
        return -1;
    }
    
    ret = initDecoding();
    if (ret != 0) {
        DemoLog(LOG_LEVEL_ERROR, "initDecoding fail.\n");
        return -1;
    }

    try {
        m_decodingThr = std::make_shared<std::thread>([this]() { decodingThread(); });
    } catch(...) {
        DemoLog(LOG_LEVEL_ERROR, "create decoding Thread failed.\n");
        return -1;
    }

    return 0;
}

void DecodeObj::decodingThread() 
{    
    DemoLog(LOG_LEVEL_INFO, "enter decodingThread.\n");

    while (!m_stop) {

        int ret = av_read_frame(m_pFormatCtx, m_pPkt) >= 0;
        if (ret < 0) {
            DemoLog(LOG_LEVEL_ERROR, "av_read_frame fail:%d.\n", ret);
            break;
        }
        DemoLog(LOG_LEVEL_INFO, "stream_index:%d, data:%p, size:%d, dts:%ld, duration:%ld, pos:%ld.\n", 
                m_pPkt->stream_index, m_pPkt->data, m_pPkt->size, m_pPkt->dts, m_pPkt->duration, m_pPkt->pos);

        //视频帧数据        
        if (m_pPkt->stream_index == m_videoStreamIdx) {

            if (m_pPkt->size) {
                decoding(m_pCodecCtx, m_pPkt, m_pFrame);
            }
        }

        //音频帧数据
        if (m_pPkt->stream_index == m_audioStreamIdx) {
            //fwrite(pkt.data, 1, pkt.size, audio_dst_file);
        }
        
    }
        
    DemoLog(LOG_LEVEL_INFO, "exit decodingThread.\n");
    return;
}

int DecodeObj::decoding(AVCodecContext *dec_ctx, AVPacket *pkt, AVFrame *frame)
{
    char buf[1024];
    int ret;

    //调试保存视频帧
    //saveH26X(m_pVideoStream, pkt);

    ret = avcodec_send_packet(dec_ctx, pkt);
    if (ret < 0) {
        DemoLog(LOG_LEVEL_ERROR, "Error sending a packet for decoding\n");
        return -1;
    }

    while (ret >= 0) {
        ret = avcodec_receive_frame(dec_ctx, frame);
        if (ret == AVERROR(EAGAIN)) {
            DemoLog(LOG_LEVEL_WARN, "during decoding, EAGAIN:%d.\n", ret);
            return ret;
        } else if (ret == AVERROR_EOF) {
            DemoLog(LOG_LEVEL_WARN, "during decoding, AVERROR_EOF:%d.\n", ret);
            return ret;
        } else if (ret < 0) {
            DemoLog(LOG_LEVEL_ERROR, "during decoding, unknow Error:%d.\n", ret);
            return ret;
        }

        //调试保存YUV
        //saveYuv(frame);

        //yuv -> rgb 
        AV_FRAME_DATA_PTR pData = yuv2rgb(dec_ctx->pix_fmt, frame);
        AvDataFormat* pInfo = pData->getPtr();
        DemoLog(LOG_LEVEL_DEBUG, "## W:%d x H:%d, len:%d.\n", pInfo->vf.width, pInfo->vf.height, pInfo->frameLen);

        //write yuv to queue
        m_pDateQue->putData(pData);
        //std::this_thread::sleep_for(std::chrono::seconds(1));
        std::this_thread::sleep_for(std::chrono::milliseconds(40));
    }

    return 0;
}

int DecodeObj::initDemuxing(const char *src_filename)
{
    int ret = 0;

    //打开输入文件，并分配 AVFormatContext
    if (avformat_open_input(&m_pFormatCtx, src_filename, NULL, NULL) < 0) {
        DemoLog(LOG_LEVEL_ERROR, "Could not open source file %s\n", src_filename);
        return -1;
    }

    //检索获取流信息
    if (avformat_find_stream_info(m_pFormatCtx, NULL) < 0) {
        DemoLog(LOG_LEVEL_ERROR, "Could not find stream information\n");
        return -1;
    }

    //在文件中找到“最佳”流，即最匹配的流。非负数表示成功，返回此流id。
    ret = av_find_best_stream(m_pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if (ret < 0) {
        DemoLog(LOG_LEVEL_ERROR, "Could not find %s stream in input file '%s'\n",
                av_get_media_type_string(AVMEDIA_TYPE_VIDEO), src_filename);
        return -1;
    }
    m_videoStreamIdx = ret;
    DemoLog(LOG_LEVEL_INFO, "video_stream_idx:%d.\n", m_videoStreamIdx);
    
    ret = av_find_best_stream(m_pFormatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
    if (ret < 0) {
        DemoLog(LOG_LEVEL_ERROR, "Could not find %s stream in input file '%s'\n",
                av_get_media_type_string(AVMEDIA_TYPE_AUDIO), src_filename);
        return -1;
    }
    m_audioStreamIdx = ret;
    DemoLog(LOG_LEVEL_INFO, "audio_stream_idx:%d.\n", m_audioStreamIdx);

    //打印有关媒体格式的详细信息，例如持续时间，比特率，流，容器，程序，元数据，辅助数据，编解码器和时基等。
    av_dump_format(m_pFormatCtx, 0, src_filename, 0);

    //音视频 AVStream
    m_pVideoStream = m_pFormatCtx->streams[m_videoStreamIdx];
    m_pAudioStream = m_pFormatCtx->streams[m_audioStreamIdx];
    if (!m_pAudioStream && !m_pVideoStream) {
        DemoLog(LOG_LEVEL_ERROR, "Could not find audio or video stream in the input, aborting\n");
        return -1;
    }
    if (m_pVideoStream) DemoLog(LOG_LEVEL_INFO, "Demuxing video from file '%s' \n", src_filename);
    if (m_pAudioStream) DemoLog(LOG_LEVEL_INFO, "Demuxing audio from file '%s' \n", src_filename);

    // AVPacket 已压缩的数据
    m_pPkt = av_packet_alloc();
    if (!m_pPkt) {
        DemoLog(LOG_LEVEL_ERROR, "av_packet_alloc fail.\n");
        return -1;
    }

    // AVFrame 未压缩的数据
    m_pFrame = av_frame_alloc();
    if (!m_pFrame) {
        DemoLog(LOG_LEVEL_ERROR, "av_frame_alloc fail.\n");
        return -1;
    }

    return 0;
}

int DecodeObj::uninitDemuxing()
{
    printf("enter uninitDemuxing.\n");
    if (m_pFormatCtx) avformat_close_input(&m_pFormatCtx);
    if (m_pFrame) av_frame_free(&m_pFrame);
    if (m_pPkt) av_packet_free(&m_pPkt);
    printf("exit uninitDemuxing.\n");
    return 0;
}

int DecodeObj::initDecoding()
{
    // AVCodec 编解码器
    m_pCodec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (!m_pCodec) {
        printf("Codec not found\n");
        return -1;
    }

    // AVCodecContext 编解码器内容
    m_pCodecCtx = avcodec_alloc_context3(m_pCodec);
    if (!m_pCodecCtx) {
        printf("Could not allocate video codec context\n");
        return -1;
    }

    /* For some codecs, such as msmpeg4 and mpeg4, width and height
       MUST be initialized there because this information is not
       available in the bitstream. */

    // open AVCodec 打开编解码器
    if (avcodec_open2(m_pCodecCtx, m_pCodec, NULL) < 0) {
        printf("Could not open codec\n");
        return -1;
    }

    // AVCodecParserContext
    m_pParserCtx = av_parser_init(m_pCodec->id);
    if (!m_pParserCtx) {
        printf("parser not found\n");
        return -1;
    }
    
    return 0;
}

int DecodeObj::uninitDecoding()
{
    printf("enter uninitDecoding.\n");
    if (m_pParserCtx) av_parser_close(m_pParserCtx);
    if (m_pCodecCtx) avcodec_free_context(&m_pCodecCtx);
    printf("exit uninitDecoding.\n");
    
    return 0;
}


}


