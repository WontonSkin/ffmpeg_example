#include <iostream>
#include "include/demo_pch.h"

using namespace std;

int main()
{
    cout << "hi" << endl;
    int ret = 0;
    ret = demo_demuxing("resources/demo.ts", "demo.h264", "demo.aac");
    ret = doRendering();
    
    return 0;
}


