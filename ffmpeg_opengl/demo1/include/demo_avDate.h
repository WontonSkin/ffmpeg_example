#ifndef __DEMO_AV_DATE_HH__
#define __DEMO_AV_DATE_HH__


#include <iostream>
#include <string>
#include <memory>
#include <mutex>
#include <queue>

namespace DEMO
{

//帧编码类型
typedef enum {
    AV_NO_CODEC = 0,
    AV_CODEC_H264 = 1,
    AV_CODEC_H265,
    AV_CODEC_AAC
}AvCodecID;

//帧编码类型
typedef enum {
    AV_CODEC_NO_TYPE = 0,
    AV_CODEC_VIDEO = 1,
    AV_CODEC_AUDIO
}AvCodecType;

//视频帧信息
typedef struct{
    bool isKey;
    unsigned int width;
    unsigned int height;
    unsigned short framerate; 
    int64_t duration;  //相对time_base而言
    int64_t time_base;
}VideoframeInfo;

//音频帧信息
typedef struct{
    unsigned int samplesRate;     //采样频率，值16000代表16k
    unsigned short channelNumber; //声道数
    unsigned short bitsPerSample; //位深
    int samplePerFrame;           //每帧采样点数
}AudioframeInfo;

//数据格式
typedef struct {
    AvCodecType type;   //帧类型
    VideoframeInfo vf;  //视频帧信息
    AudioframeInfo af;  //音频帧信息
    unsigned int frameLen;      //帧长度
    unsigned char *frameBuf;    //帧内容
} AvDataFormat;

class AvFrameData {
public:
    AvFrameData(unsigned int frameLen);

    AvFrameData(AvFrameData& other);
        
    virtual ~AvFrameData();

    AvDataFormat* getPtr();

private:
    AvFrameData& operator = (AvFrameData& data); 

private:
    AvDataFormat* m_pAvDataFormat;
};

typedef std::shared_ptr<AvFrameData> AV_FRAME_DATA_PTR;


class AvDataQueue {
public:
    AvDataQueue();
    virtual ~AvDataQueue();
private:
    AvDataQueue(AvDataQueue& o);
    AvDataQueue& operator = (AvDataQueue& o);
    
public:
    bool bEmptyQueue();
    int clearQueue();
    unsigned int getQueueSize();
    int putData(AV_FRAME_DATA_PTR& pData);
    AV_FRAME_DATA_PTR getData();

private:
    std::mutex m_Mtx;
    std::queue<AV_FRAME_DATA_PTR> m_Que;
};



}


#endif
