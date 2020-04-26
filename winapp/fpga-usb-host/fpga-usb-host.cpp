// fpga-usb-host.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdint.h>
#include <windows.h>
#include <chrono>
#include <map>

#include "CUsbHost.h";

using namespace std;

int main()
{
	CUsbHost host;
	host.init(L"\\\\.\\COM18",921600);

	while (1)
		Sleep(1000);

	return 0;
}
