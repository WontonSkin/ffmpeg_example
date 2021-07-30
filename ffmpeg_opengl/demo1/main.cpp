#include <iostream>       // std::cout
#include <thread>         // std::thread
#include <chrono>         // std::chrono::seconds

#include "demo_avDate.h"
#include "demo_decoding.h"
#include "demo_rendering.h" 


int main()
{
  


    //资源申请
    DEMO::AvDataQueue* pDateQue = NULL;
    DEMO::DecodeObj* pDecoder = NULL;
    DEMO::RenderObj* pRender = NULL;
    try {
        pDateQue = new DEMO::AvDataQueue;
        pDecoder = new DEMO::DecodeObj(pDateQue);
        pRender = new DEMO::RenderObj(pDateQue);
    } catch(...) {
        std::cout << "new (AvDataQueue/DecodeObj/RenderObj) fail.\n";
    }

    //wait do stuff...
    while(1) {
        //std::this_thread::sleep_for(std::chrono::milliseconds(10));
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }

    //资源释放
    if (pRender != NULL) {
        delete pRender;
    }
    if (pDecoder != NULL) {
        delete pDecoder;
    }
    if (pDateQue != NULL) {
        delete pDateQue;
    }

    return 0;
}

