#include <iostream>
#include <string>

#include "demo_rendering.h"


namespace DEMO
{

RenderObj::RenderObj(AvDataQueue* pDateQue) : m_pDateQue(pDateQue)
{
    m_stop = false;
}

RenderObj::~RenderObj()
{
	if (m_stop != true) {
		m_stop = true;
		if (m_renderingThr.get() != nullptr) {
			std::cout << "stop renderingThread.\n";
			m_renderingThr->join();
		}
	}

    std::cout << "~RenderObj suc.\n";
}

int RenderObj::init()
{
	try {
		m_renderingThr = std::make_shared<std::thread>([this]() { renderingThread(); });
	}
	catch (...) {
		std::cout << "create AvConsumer Thread failed.\n";
		return -1;
	}
}

void RenderObj::renderingThread() 
{
    std::cout << "enter renderingThread.\n";

    while (!m_stop) {
        
    }

    std::cout << "exit renderingThread.\n";
}


}

