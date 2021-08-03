#include <iostream>       // std::cout
#include <thread>         // std::thread
#include <chrono>         // std::chrono::seconds

#include "demo_avDate.h"
#include "demo_decoding.h"
#include "demo_rendering.h" 
#include "demo_log.h"

#include <Windows.h>


int main()
{
    //DEMO::initlog();
    DEMO::openLogFile();
    
    int ret = 0;
    std::string url("resources/demo.ts");

    //清除残留文件
    DeleteFile("raw.h264");  //include Windows.h
    DeleteFile("raw.yuv");

    //资源申请
    DEMO::AvDataQueue* pDateQue = NULL;
    DEMO::DecodeObj* pDecoder = NULL;
    DEMO::RenderObj* pRender = NULL;
    try {
        pDateQue = new DEMO::AvDataQueue;
        pDecoder = new DEMO::DecodeObj(pDateQue, url);
        pRender = new DEMO::RenderObj(pDateQue);
    } catch(...) {
        std::cout << "new (AvDataQueue/DecodeObj/RenderObj) fail.\n";
    }

    //初始化资源
    ret = pDecoder->init();
    if (ret != 0) {
        std::cout << "pDecoder->init fail.\n";
        return -1;
    }
    ret = pRender->init();
    if (ret != 0) {
        std::cout << "pRender->init fail.\n";
        return -1;
    }

    //wait do stuff...
    while (1) {
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

    DEMO::closeLogfile();

    return 0;
}

