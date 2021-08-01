#include <string.h>
#include "demo_avDate.h"

namespace DEMO
{


AvFrameData::AvFrameData() 
{
    memset(&m_avDataFormat, 0, sizeof(m_avDataFormat));
    m_pdata = NULL;
    m_len = 0;
}

AvFrameData::AvFrameData(AvFrameData& data) 
{
    m_avDataFormat = data.getAvDataFormat();
    m_len = data.getDataLen();
    
    if ((m_len != 0) && (data.getData() != NULL)) {
        m_pdata = new unsigned char[m_len];
        memcpy(m_pdata, data.getData(), m_len);
    } else {
        m_len = 0;
        m_pdata = NULL;
    }
}

AvFrameData::AvFrameData(AvDataFormat& avformat) : m_avDataFormat(avformat) 
{
    if ((avformat.frameBuf != NULL) && (avformat.frameLen != 0)) {
        m_pdata = new unsigned char[avformat.frameLen];
        memcpy(m_pdata, avformat.frameBuf, avformat.frameLen);
        m_len = avformat.frameLen;

        //实际数据已拷贝，故清一下
        m_avDataFormat.frameBuf = NULL;
        m_avDataFormat.frameLen = 0;
    } else {
        memset(&m_avDataFormat, 0, sizeof(m_avDataFormat));
        m_len = 0;
        m_pdata = NULL;
    }
}

AvFrameData& AvFrameData::operator = (AvFrameData& data) 
{
    if (this != &data) {
        
        if (0 == data.getDataLen() || NULL == data.getData()) {
            memset(&m_avDataFormat, 0, sizeof(m_avDataFormat));
            m_pdata = NULL;
            m_len = 0;
            
        } else {
    
            if (m_len < data.getDataLen()) {
                delete[] m_pdata;
                m_pdata = new unsigned char[data.getDataLen()];
            }
            
            memcpy(m_pdata, data.getData(), data.getDataLen());
            m_avDataFormat = data.getAvDataFormat();
            m_len = data.getDataLen();
        }
    }
    return *this;
}

AvFrameData::~AvFrameData() {
    if (m_pdata && (m_len > 0)) {
        delete[] m_pdata;
    }
}


AvDataQueue::AvDataQueue()
{

}

AvDataQueue::~AvDataQueue()
{

}

bool AvDataQueue::bEmptyQueue()
{
    std::unique_lock<std::mutex> lck(m_Mtx);
    return m_Que.empty();
}

int AvDataQueue::clearQueue()
{
    std::unique_lock<std::mutex> lck(m_Mtx);
    while (!m_Que.empty()) {
        m_Que.pop();
    }
    return 0;
}

unsigned int AvDataQueue::getQueueSize()
{
    std::unique_lock<std::mutex> lck(m_Mtx);
    return m_Que.size();
}

int AvDataQueue::putData(AV_FRAME_DATA_PTR& pData)
{
    std::unique_lock<std::mutex> lck(m_Mtx);
    m_Que.push(pData);
    return 0;
}

AV_FRAME_DATA_PTR AvDataQueue::getData()
{
    AV_FRAME_DATA_PTR pData;
        
    std::unique_lock<std::mutex> lck(m_Mtx);
    if (m_Que.empty()) {
        
    } else {
        AV_FRAME_DATA_PTR pData = m_Que.front();
        m_Que.pop();
    }

    return pData;
}


}


