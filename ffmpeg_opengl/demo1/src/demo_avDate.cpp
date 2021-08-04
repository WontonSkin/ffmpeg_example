#include <string.h>
#include "demo_avDate.h"
#include "demo_log.h"


namespace DEMO
{

AvFrameData::AvFrameData(unsigned int frameLen)
{
    m_pAvDataFormat = new AvDataFormat;
    memset(m_pAvDataFormat, 0, sizeof(*m_pAvDataFormat));
    if (frameLen != 0) {
        m_pAvDataFormat->frameBuf = new unsigned char[frameLen];
    } else {
        m_pAvDataFormat->frameBuf = NULL;
    }
}


AvFrameData::AvFrameData(AvFrameData& other) 
{
    AvDataFormat* ptr = other.getPtr();
    if (NULL == ptr) {
        m_pAvDataFormat = NULL;
    } else {
    
        m_pAvDataFormat = new AvDataFormat;
        memcpy(m_pAvDataFormat, ptr, sizeof(*ptr));
        if (ptr->frameLen) {
            m_pAvDataFormat->frameBuf = new unsigned char[ptr->frameLen];
            memcpy(m_pAvDataFormat->frameBuf, ptr->frameBuf, ptr->frameLen);
        } else {
            m_pAvDataFormat->frameBuf = NULL;
        }
        
    }
}

/**
AvFrameData& AvFrameData::operator = (AvFrameData& other) 
{
    if (this != &other) {

        AvDataFormat* ptr = other.getPtr();
        if (NULL == ptr) {
            if (m_pAvDataFormat) {
                if (m_pAvDataFormat->frameBuf) {
                    delete[] m_pAvDataFormat->frameBuf;
                }
                delete m_pAvDataFormat;
                //m_pAvDataFormat = NULL;
            }
            m_pAvDataFormat = NULL;
        } else {
        
            if (m_pAvDataFormat) {
                
                m_pAvDataFormat->type = ptr->type;
                m_pAvDataFormat->vf = ptr->vf;
                m_pAvDataFormat->af = ptr->af;
                if (m_pAvDataFormat->frameLen < ptr->frameLen) {
                    delete[] m_pAvDataFormat->frameBuf;
                    m_pAvDataFormat->frameBuf = new unsigned char[ptr->frameLen];
                }
                memcpy(m_pAvDataFormat->frameBuf, ptr->frameBuf, ptr->frameLen);
                m_pAvDataFormat->frameLen = ptr->frameLen;
                    
            } else {
                m_pAvDataFormat = new AvDataFormat;
                memcpy(m_pAvDataFormat, ptr, sizeof(*ptr));
                m_pAvDataFormat->frameBuf = new unsigned char[ptr->frameLen];
                memcpy(m_pAvDataFormat->frameBuf, ptr->frameBuf, ptr->frameLen);
            }
        }
    }
    return *this;
}
**/

AvFrameData::~AvFrameData() {
    if (m_pAvDataFormat) {
        if (m_pAvDataFormat->frameBuf) {
            delete[] m_pAvDataFormat->frameBuf;
        }
        delete m_pAvDataFormat;
        m_pAvDataFormat = NULL;
    }
}

AvDataFormat* AvFrameData::getPtr() 
{    
    return m_pAvDataFormat;
}



AvDataQueue::AvDataQueue()
{

}

AvDataQueue::~AvDataQueue()
{
	clearQueue();
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
        pData = m_Que.front();
        m_Que.pop();
    }

    return pData;
}


}


