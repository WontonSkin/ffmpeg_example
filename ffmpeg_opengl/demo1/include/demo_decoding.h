#ifndef __DEMO_DECODING_HH__
#define __DEMO_DECODING_HH__

#include <iostream>
#include <string>
#include <memory>
#include <mutex>
#include <queue>

#include "demo_avDate.h"

extern "C"
{
#include "libavutil/timestamp.h"
#include "libavutil/opt.h"
#include "libavutil/channel_layout.h"
#include "libavutil/common.h"
#include "libavutil/imgutils.h"
#include "libavutil/mathematics.h"
#include "libavutil/samplefmt.h"
#include "libavutil/time.h"
#include "libavutil/fifo.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavformat/avio.h"
#include "libavfilter/avfilter.h"
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
}


namespace DEMO
{
    
void decodingThread(int x);

class DecodeObj {
public:
    DecodeObj(AvDataQueue* pDateQue, std::string url);
    virtual ~DecodeObj();
    int init();

private:
    DecodeObj(DecodeObj& obj);
    DecodeObj& operator = (DecodeObj& obj);

    void decodingThread();
    int decoding(AVCodecContext *dec_ctx, AVPacket *pkt, AVFrame *frame);
    int initDemuxing(const char *src_filename);
    int uninitDemuxing();
    int initDecoding();
    int uninitDecoding();

private:
    bool m_stop;
    std::string m_url;
    AvDataQueue* m_pDateQue;
    std::shared_ptr<std::thread> m_decodingThr;

    //demuxing
    AVFormatContext *m_pFormatCtx;
    AVStream *m_pVideoStream;
    AVStream *m_pAudioStream;
    int m_videoStreamIdx;
    int m_audioStreamIdx;
    AVPacket *m_pPkt;
    AVFrame *m_pFrame;

    //decoding
	const AVCodec *m_pCodec;
    AVCodecParserContext *m_pParserCtx;
    AVCodecContext *m_pCodecCtx;
};

}

#endif
