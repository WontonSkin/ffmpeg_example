#include <iostream>
#include "include/demo_pch.h"

using namespace std;

int main()
{
	cout << "hi" << endl;
	int ret = demo_demuxing("demo.ts", "demo.h264", "demo.aac");
	return 0;
}


