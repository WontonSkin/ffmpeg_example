#ifndef __DEMO_RENDERING_HH__
#define __DEMO_RENDERING_HH__

#include <iostream>
#include <string>
#include <memory>
#include <mutex>
#include <queue>

#include "demo_avDate.h"

namespace DEMO
{

void renderingThread(int x);


class  RenderObj {
public:
    RenderObj(AvDataQueue* pDateQue);
    virtual ~RenderObj();
	int init();

private:
    RenderObj(RenderObj& obj);
    RenderObj& operator = (RenderObj& obj);

public:
    void renderingThread();

private:
    bool m_stop;
    AvDataQueue* m_pDateQue;
	std::shared_ptr<std::thread> m_renderingThr;
};


}


#endif
